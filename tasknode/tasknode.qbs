import qbs

QtApplication {
    cpp.cxxLanguageVersion: "c++20"

    cpp.defines: [
        // You can make your code fail to compile if it uses deprecated APIs.
        // In order to do so, uncomment the following line.
        //"QT_DISABLE_DEPRECATED_BEFORE=0x060000" // disables all the APIs deprecated before Qt 6.0.0
    ]

    consoleApplication: true
    files: [
        "../common/logMessageHandler.cpp",
        "../common/transportService.h",
        "../common/transportService.cpp",
        "tasknode.h",
        "tasknode.cpp",
        "main.cpp",
    ]

    Group {     // Properties for the produced executable
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }

    Depends {name: "Qt.network" }
}
