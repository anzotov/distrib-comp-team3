#include "../common/logMessageHandler.h"
#include "tasknode.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QStringList>
#include <QtGlobal>

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logMessageHandler);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("tasknode");

    QCommandLineParser parser;
    parser.setApplicationDescription("Программа для выдачи задания вычислительному кластеру");
    parser.addHelpOption();
    parser.addPositionalArgument("<ip>:<port>", "IP-адрес и порт вычислительного узла, к которому будет выполняться подключение");
    parser.addOption({"i", "Чтение задачи из файла <input_file> или стандартного ввода", "{<input_file>|-}"});
    parser.addOption({"o", "Запись результата в файл <output_file> или стандартный вывод", "{<output_file>|-}"});
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

    const QStringList args = parser.positionalArguments();
    if (args.length() != 1)
    {
        qFatal("Укажите один позиционный аргумент: <ip>:<port>");
    }
    auto input = parser.value("i");
    if (input.isEmpty())
    {
        qFatal("Укажите значение для опции -i");
    }
    auto output = parser.value("o");
    if (output.isEmpty())
    {
        qFatal("Укажите значение для опции -o");
    }
    auto split = args.at(0).split(':');
    if (split.length() != 2 || split[0].isEmpty() || split[1].isEmpty())
    {
        qFatal("Укажите один позиционный аргумент: <ip>:<port>");
    }

    TaskNode taskNode(&app, split[0], split[1], input, output);
    QObject::connect(&taskNode, &TaskNode::taskDone, &app, [&app](bool success)
                     { app.exit(success ? 0 : 1); });

    return app.exec();
}
