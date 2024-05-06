import qbs

Project
{
    AutotestRunner { }

    QtApplication {
        name: "compnode-tests"
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
            "compnode-tests.cpp",
            "compnode-tests.h",
            "../compnode/peerServiceBase.h",
            "../compnode/chunkerServiceBase.h",
            "../compnode/compnode.h",
            "../compnode/compnode.cpp",
            "../common/calcResult.cpp",
            "../common/calcTask.cpp",
        ]

        Group {     // Properties for the produced executable
            fileTagsFilter: "application"
            qbs.install: true
            qbs.installDir: "bin"
        }

        Depends { name: "Qt.testlib" }
    }

    QtApplication {
        name: "jsCalculatorService-tests"
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
            "jsCalculatorService-tests.cpp",
            "jsCalculatorService-tests.h",
            "../compnode/calculatorServiceBase.h",
            "../compnode/jsCalculatorService.h",
            "../compnode/jsCalculatorService.cpp",
            "../common/calcResult.cpp",
            "../common/calcTask.cpp",
        ]

        Group {     // Properties for the produced executable
            fileTagsFilter: "application"
            qbs.install: true
            qbs.installDir: "bin"
        }

        Depends { name: "Qt.testlib" }
        Depends { name: "Qt.qml" }
    }
    QtApplication {
        name: "chunkerService-tests"
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
            "chunkerService-tests.cpp",
            "chunkerService-tests.h",
            "../compnode/chunkerServiceBase.h",
            "../compnode/chunkerService.h",
            "../compnode/chunkerService.cpp",
            "../common/calcResult.cpp",
            "../common/calcTask.cpp",
            "../compnode/calculatorServiceBase.h",
        ]

        Group {     // Properties for the produced executable
            fileTagsFilter: "application"
            qbs.install: true
            qbs.installDir: "bin"
        }

        Depends { name: "Qt.testlib" }
    }
}