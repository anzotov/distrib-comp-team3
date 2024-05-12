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
    qDebug() << "ChunkerService: start()";
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
    qDebug() << QStringLiteral("ChunkerService: calculateTask(%1)").arg(peerHandler);
    if (m_state != State::Ready)
        return;

    m_taskSourceHandler = peerHandler;
    if (!task.isMain)
    {
        qInfo() << "Calculating chunked task";
        m_state = State::ChunkTaskReceived;
        m_calculatorService->calculate(task);
        return;
    }

    qInfo() << "Calculating task";
    m_state = State::MainTaskReceived;
    m_result.reset();
    m_resultChunks.clear();
    m_resultChunksOrder.clear();
    m_peers.empty() ? m_calculatorService->calculate(task) : sliceTask(task);
}

void ChunkerService::sliceTask(const CalcTask &task)
{
    qDebug() << "ChunkerService: sliceTask()";
    QMap<PeerHandlerType, double> chunkCoefficients;
    double myCompPower = m_compPower.toDouble();
    qDebug() << "My computational power: " << m_compPower;
    for (const auto &peer : m_peers)
    {
        qDebug() << QStringLiteral("Peer %1 computational power: %2").arg(peer.peerHandler).arg(peer.compPower);
        double powerCoef = myCompPower / peer.compPower.toDouble();
        chunkCoefficients.insert(peer.peerHandler, powerCoef);
    }
    double normCoef = std::accumulate(chunkCoefficients.cbegin(), chunkCoefficients.cend(), 1);

    auto dataSize = task.data.size();
    CalcTask chunk(task.function, {}, false);
    int curItem = 0;
    for (const auto &peer : chunkCoefficients.keys())
    {
        int i = curItem;
        for (; (i < dataSize) && (i < curItem + int(dataSize * chunkCoefficients[peer] / normCoef)); ++i)
        {
            chunk.data.append(task.data.at(i));
        }
        m_resultChunks.insert(peer, {});
        m_resultChunksOrder.append(peer);
        qDebug() << QStringLiteral("Sending chunked task (%1 data entries) to: %2").arg(chunk.data.size()).arg(peer);
        emit sendChunkedTask(peer, chunk);
        curItem = i;
        chunk.data.clear();
    }
    for (; curItem < dataSize; ++curItem)
    {
        chunk.data.append(task.data.at(curItem));
    }
    qDebug() << QStringLiteral("Calculating my chunk (%1 data entries)").arg(chunk.data.size());
    m_calculatorService->calculate(chunk);
}

void ChunkerService::updatePeers(const QList<PeerInfo> &peers)
{
    qDebug() << "ChunkerService: updatePeers()";
    QMap<PeerHandlerType, PeerInfo> peersMap;
    for (const auto &peer : peers)
    {
        peersMap.insert(peer.peerHandler, peer);
    }
    bool isPeerMissing = false;
    if (m_state == State::MainTaskReceived)
    {
        for (const auto &peer : m_resultChunks.keys())
        {
            if (!peersMap.contains(peer))
            {
                qDebug() << QStringLiteral("Peer %1 disconnected, calculation can't be completed").arg(peer);
                isPeerMissing = true;
                break;
            }
        }
    }
    else if (m_state == State::ChunkTaskReceived && !peersMap.contains(m_taskSourceHandler))
    {
        qDebug() << QStringLiteral("Task source peer %1 disconnected").arg(m_taskSourceHandler);
        isPeerMissing = true;
    }
    m_peers = peersMap;
    if (isPeerMissing)
    {
        qCritical() << "Peer(s) disconnected, calculation can't be completed";
        stopCalc();
    }
}

void ChunkerService::OnCalculatorServiceCalcDone(CalcResult result)
{
    qDebug() << "ChunkerService: OnCalculatorServiceCalcDone()";
    if (m_state == State::TestCalcRequested)
    {
        std::chrono::nanoseconds taskTime = std::chrono::steady_clock::now() - m_taskStartTime;
        m_compPower = QStringLiteral("%1").arg(taskTime.count());
        qDebug() << "My computational power:" << m_compPower;
        setStateReady();
        return;
    }
    if (m_state == State::ChunkTaskReceived)
    {
        qInfo() << "Chunked task calculated";
        m_state = State::Ready;
        emit calcResult(m_taskSourceHandler, result);
        return;
    }
    if (m_state == State::MainTaskReceived)
    {
        qInfo() << "My task chunk calculated";
        m_result = result;
        checkChunkedResult();
    }
}

void ChunkerService::processChunkedResult(const PeerHandlerType &peerHandler, const CalcResult &result)
{
    qDebug() << QStringLiteral("ChunkerService: processChunkedResult(%1)").arg(peerHandler);
    if (result.isMain == true)
    {
        qCritical() << "Wrong result type received";
        stopCalc();
    }
    else
    {
        m_resultChunks[peerHandler] = result;
        checkChunkedResult();
    }
}

void ChunkerService::checkChunkedResult()
{
    qDebug() << "ChunkerService: checkChunkedResult()";
    if (!m_result.has_value())
        return;
    for (const auto &result : m_resultChunks)
    {
        if (!result.has_value())
            return;
    }
    qInfo() << "ChunkerService: all chunks ready";
    CalcResult result({}, true);
    for (const auto &peer : m_resultChunksOrder)
    {
        result.data.append(m_resultChunks[peer]->data);
    }
    result.data.append(m_result->data);
    m_state = State::Ready;
    emit calcResult(m_taskSourceHandler, result);
}

void ChunkerService::stopCalc()
{
    qDebug() << "ChunkerService: stopCalc()";
    qInfo() << "Stopping calculation";
    m_state = State::Ready;
    m_calculatorService->stop();
    emit calcError();
}

void ChunkerService::setStateReady()
{
    qDebug() << "ChunkerService: setStateReady()";
    m_state = State::Ready;
    m_taskSourceHandler = "";
    emit ready(m_compPower);
}

void ChunkerService::stop()
{
    qDebug() << "ChunkerService: stop()";
    m_state = State::Stopped;
    m_calculatorService->stop();
}
