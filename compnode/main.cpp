#include "../common/logMessageHandler.h"
#include "compnode.h"
#include "peerService.h"
#include "discoveryService.h"
#include "chunkerService.h"
#include "jsCalculatorService.h"
#include "../common/transportService.h"
#include "../common/jsonSerializer.h"
#include "../common/compressor.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QStringList>
#include <QUuid>
#include <QNetworkInterface>

using namespace std::literals;

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logMessageHandler);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("compnode");

    QCommandLineParser parser;
    parser.setApplicationDescription("Программа вычислительного узла");
    parser.addHelpOption();
    parser.addOption({"u", "Порт для multicast рассылок по протоколу UDP", "udp_port"});
    parser.addOption({"t", "Порт для входящих подключений по TCP. Если не задано, задается автоматически", "tcp_port"});
    parser.addOption({QStringList() << "quiet"
                                    << "q",
                      "Тихий режим, без вывода информации"});
    parser.addOption({QStringList() << "verbose"
                                    << "v",
                      "Подробный режим, с выводом дополнительной информации"});
    parser.addOption({QStringList() << "debug"
                                    << "d",
                      "Режим с выводом отладочной информации"});
    parser.process(app);
    logMessageLevel.quiet = parser.isSet("q");
    logMessageLevel.verbose = parser.isSet("v");
    logMessageLevel.debug = parser.isSet("d");

    auto udpPort = parser.value("u");
    if (udpPort.isEmpty())
    {
        qFatal("Укажите значение для опции -u");
    }
    auto tcpPort = parser.value("t");
    if (tcpPort.isEmpty())
    {
        tcpPort = "0";
    }
    quint16 tcpPortValue = tcpPort.toUInt();

    // TODO: Вписать сюда конструктор TransportLayer, использующий tcpPortValue в качестве порта для прослушивания
    TransportLayerBase *transportLayer = nullptr;
    if (tcpPortValue == 0)
    {
        // TODO: Сделать функцию, возвращающую QTcpServer::serverPort(), раскомментировать следующую строку
        // tcpPortValue = transportLayer.getPort();
    }

    QStringList connectInfo;
    for (const auto &address : QNetworkInterface::allAddresses())
    {
        if (address.isGlobal())
        {
            connectInfo.append(QStringLiteral("%1:%2").arg(address.toString()).arg(tcpPortValue));
        }
    }

    DiscoveryData discoveryData(QUuid::createUuid().toString(), connectInfo);
    auto discoveryService = new DiscoveryService(new JsonSerializer,
                                                 QHostAddress("239.255.43.21"),
                                                 udpPort.toUInt(), 5s, 10);
    auto transportService = new TransportService(transportLayer,
                                                 new JsonSerializer,
                                                 new Compressor);
    auto peerService = new PeerService(transportService, discoveryService, discoveryData);
    auto chunkerService = new ChunkerService(new JsCalculatorService);
    CompNode compNode(peerService, chunkerService);
    QObject::connect(&compNode, &CompNode::stopped, &app, [&app]()
                     { app.exit(0); });

    return app.exec();
}
