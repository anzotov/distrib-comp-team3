#include "chunkerService.h"

#include <algorithm>

ChunkerService::ChunkerService(CalculatorServiceBase *calculatorService, QObject *parent)
    : ChunkerServiceBase(parent),
      m_calculatorService(calculatorService)
{
    qDebug() << "ChunkerService()";
    if (!m_calculatorService)
    {
        throw std::runtime_error("ChunkerService: not all dependencies were satisfied");
    }
    m_testTask.function = "1";
    for (int i = 2; i < 10; ++i)
    {
        m_testTask.function += QStringLiteral("*%1").arg(i);
    }
    m_testTask.data.append("1");
    QObject::connect(m_calculatorService, &CalculatorServiceBase::calcDone, this, &ChunkerService::OnCalculatorServiceCalcDone);
}

ChunkerService::~ChunkerService()
{
    qDebug() << "~ChunkerService()";
    delete m_calculatorService;
}

void ChunkerService::start()
{
    qInfo() << "ChunkerService: start";
    if (m_state != State::Stopped)
        throw std::logic_error("ChunkerService: need to stop before starting again");
    if (!m_compPower.isEmpty())
    {
        setStateReady();
        return;
    }
    m_state = State::TestCalcRequested;
    m_taskStartTime = std::chrono::steady_clock::now();
    m_calculatorService->calculate(m_testTask);
}

void ChunkerService::calculateTask(const PeerHandlerType &peerHandler, const CalcTask &task)
{
    if (m_state != State::Ready)
        return;

    m_taskSourceHandler = peerHandler;
    if (!task.isMain)
    {
        m_state = State::ChunkTaskReceived;
        m_calculatorService->calculate(task);
        return;
    }

    m_state = State::MainTaskReceived;
    m_result.reset();
    m_resultChunks.clear();
    m_peers.empty() ? m_calculatorService->calculate(task) : sliceTask(task);
}

void ChunkerService::sliceTask(const CalcTask &task)
{
    QMap<PeerHandlerType, double> chunkCoefficients;
    double myCompPower = m_compPower.toDouble();
    for (const auto &peer : m_peers)
    {
        double powerCoef = myCompPower / peer.compPower.toDouble();
        chunkCoefficients.insert(peer.peerHandler, powerCoef);
    }
    double normCoef = std::accumulate(chunkCoefficients.cbegin(), chunkCoefficients.cend(), myCompPower);

    auto dataSize = task.data.size();
    CalcTask chunk(task.function, {}, false);
    int curItem = 0;
    for (const auto &peer : chunkCoefficients.keys())
    {
        int i = curItem;
        for (; i < curItem + int(dataSize * chunkCoefficients[peer] / normCoef); ++i)
        {
            chunk.data.append(task.data.at(curItem));
        }
        m_resultChunks.insert(peer, {});
        m_resultChunksOrder.append(peer);
        emit sendChunkedTask(peer, chunk);
        curItem = i;
        chunk.data.clear();
    }
    for (; curItem < dataSize; ++curItem)
    {
        chunk.data.append(task.data.at(curItem));
    }
    m_calculatorService->calculate(chunk);
}

void ChunkerService::updatePeers(const QList<PeerInfo> &peers)
{
    QMap<PeerHandlerType, PeerInfo> peersMap;
    for (const auto &peer : peers)
    {
        peersMap.insert(peer.peerHandler, peer);
    }
    if (m_state != State::MainTaskReceived)
    {
        m_peers = peersMap;
        return;
    }
    bool isPeerMissing = false;
    for (const auto &peer : m_peers.keys())
    {
        if (!peersMap.contains(peer))
        {
            isPeerMissing = true;
            break;
        }
    }
    m_peers = peersMap;
    if (isPeerMissing)
    {
        m_state = State::Ready;
        m_calculatorService->stop();
        emit calcError();
    }
}

void ChunkerService::OnCalculatorServiceCalcDone(CalcResult result)
{
    if (m_state == State::TestCalcRequested)
    {
        std::chrono::nanoseconds taskTime = std::chrono::steady_clock::now() - m_taskStartTime;
        m_compPower = taskTime.count();
        return;
    }
    if (m_state == State::ChunkTaskReceived)
    {
        m_state = State::Ready;
        emit CalcResult(result);
        return;
    }
    if (m_state == State::MainTaskReceived)
    {
        m_result = result;
        checkChunkedResult();
    }
}

void ChunkerService::processChunkedResult(const PeerHandlerType &peerHandler, const CalcResult &result)
{
    m_resultChunks[peerHandler] = result;
    checkChunkedResult();
}

void ChunkerService::checkChunkedResult()
{
    if (!m_result.has_value())
        return;
    for (const auto &result : m_resultChunks)
    {
        if (!result.has_value())
            return;
    }
    CalcResult result({}, true);
    for (const auto &peer : m_resultChunksOrder)
    {
        result.data.append(m_resultChunks[peer]->data);
    }
    result.data.append(m_result->data);
    m_state = State::Ready;
    emit calcResult(m_taskSourceHandler, result);
}

void ChunkerService::setStateReady()
{
    m_state = State::Ready;
    m_taskSourceHandler = "";
    emit ready(m_compPower);
}

void ChunkerService::stop()
{
    m_state = State::Stopped;
    m_calculatorService->stop();
}
