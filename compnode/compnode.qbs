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
        "main.cpp",
        "peerServiceBase.h",
        "peerService.h",
        "peerService.cpp",
        "chunkerService.h",
        "chunkerService.cpp",
        "chunkerServiceBase.h",
        "compnode.h",
        "compnode.cpp",
        "calculatorServiceBase.h",
        "jsCalculatorService.h",
        "jsCalculatorService.cpp",
        "discoveryData.h",
        "discoveryData.cpp",
        "discoveryServiceBase.h",
        "discoveryService.h",
        "discoveryService.cpp",
        "../common/logMessageHandler.cpp",
        "../common/transportService.h",
        "../common/transportService.cpp",
        "../common/transportServiceBase.h",
        "../common/calcResult.cpp",
        "../common/calcTask.cpp",
        "../common/handshake.cpp",
        "../common/jsonSerializer.h",
        "../common/jsonSerializer.cpp",
        "../common/serializerBase.h",
        "../common/encoderBase.h",
        "../common/compressor.h",
        "../common/compressor.cpp",
        "../common/transportLayerBase.h",
        "../common/transportLayer.h",
        "../common/transportLayer.cpp"
    ]

    Group {
        // Properties for the produced executable
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }

    Depends {
        name: "Qt.qml"
    }
    Depends {
        name: "Qt.network"
    }
}
