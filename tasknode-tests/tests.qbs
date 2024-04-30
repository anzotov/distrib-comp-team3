import qbs

Project
{
    AutotestRunner { }

    QtApplication {
        name: "tasknode-tests"
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
            "tasknode-tests.cpp",
            "tasknode-tests.h",
            "../common/transportServiceBase.h",
            "../common/calcResult.cpp",
            "../common/calcTask.cpp",
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
    }

    QtApplication {
        name: "fileTaskProvider-tests"
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
            "fileTaskProvider-tests.cpp",
            "fileTaskProvider-tests.h",
            "../tasknode/fileTaskProvider.h",
            "../tasknode/fileTaskProvider.cpp",
            "../tasknode/taskProvider.h",
            "../common/calcResult.cpp",
        ]

        Group {     // Properties for the produced executable
            fileTagsFilter: "application"
            qbs.install: true
            qbs.installDir: "bin"
        }

        Depends { name: "Qt.testlib" }
    }
}