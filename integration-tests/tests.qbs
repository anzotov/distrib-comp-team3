import qbs

Project
{
    AutotestRunner { }

    QtApplication {
        name: "no-network-tests"
        type: base.concat("autotest")

        cpp.cxxLanguageVersion: "c++20"

        cpp.defines: [
            // You can make your code fail to compile if it uses deprecated APIs.
            // In order to do so, uncomment the following line.
            //"QT_DISABLE_DEPRECATED_BEFORE=0x060000" // disables all the APIs deprecated before Qt 6.0.0
        ]

        cpp.treatWarningsAsErrors: true

        consoleApplication: true
        files: [
            "no-network-tests.h",
            "no-network-tests.cpp",
            "networkMock.h",
            "networkMock.cpp",
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
            "../compnode/peerServiceBase.h",
            "../compnode/peerService.h",
            "../compnode/peerService.cpp",
            "../compnode/chunkerService.h",
            "../compnode/chunkerService.cpp",
            "../compnode/chunkerServiceBase.h",
            "../compnode/compnode.h",
            "../compnode/compnode.cpp",
            "../compnode/calculatorServiceBase.h",
            "../compnode/jsCalculatorService.h",
            "../compnode/jsCalculatorService.cpp",
            "../compnode/discoveryData.h",
            "../compnode/discoveryData.cpp",
            "../compnode/discoveryServiceBase.h",
            "../tasknode/tasknode.h",
            "../tasknode/tasknode.cpp",
            "../tasknode/taskProvider.h",
        ]

        Group {     // Properties for the produced executable
            fileTagsFilter: "application"
            qbs.install: true
            qbs.installDir: "bin"
        }

        Depends { name: "Qt.testlib" }
        Depends { name: "Qt.qml" }
    }
}