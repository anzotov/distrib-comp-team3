import qbs

Project
{
    AutotestRunner { }

    QtApplication {
        name: "common-tests"
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
            "common-tests.cpp",
            "common-tests.h",
            "../common/jsonSerializer.cpp",
            "../common/calcTask.cpp",
            "../common/calcResult.cpp",
            "../common/handshake.cpp",
        ]

        Group {     // Properties for the produced executable
            fileTagsFilter: "application"
            qbs.install: true
            qbs.installDir: "bin"
        }

        Depends { name: "Qt.testlib" }
    }
    QtApplication {
        name: "transportService-tests"
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
            "transportService-tests.cpp",
            "transportService-tests.h",
            "../common/transportLayerBase.h",
            "../common/transportServiceBase.h",
            "../common/transportService.h",
            "../common/transportService.cpp",
            "../common/jsonSerializer.cpp",
            "../common/calcResult.cpp",
            "../common/calcTask.cpp",
            "../common/handshake.cpp",
        ]

        Group {     // Properties for the produced executable
            fileTagsFilter: "application"
            qbs.install: true
            qbs.installDir: "bin"
        }

        Depends { name: "Qt.testlib" }
    }
}