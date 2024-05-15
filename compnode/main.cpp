#include "../common/logMessageHandler.h"
#include "compnode.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QStringList>
#include <QtGlobal>

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logMessageHandler);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("compnode");

    QCommandLineParser parser;
    parser.setApplicationDescription("Программа вычислительного узла");
    parser.addHelpOption();
    parser.addOption({"u", "Порт для multicast рассылок по протоколу UDP <udp_port>}"});
    parser.addOption({"t", "Порт для входящих подключений по TCP <tcp_port>. Если не задано, задается автоматически"});
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


    return app.exec();
}
