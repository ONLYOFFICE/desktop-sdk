QT  += core gui widgets printsupport

TEMPLATE = app
CONFIG += app_bundle

TARGET = IvolgaPRO
mac {
TARGET = ONLYOFFICE
}

CONFIG += c++11

CONFIG -= debug_and_release debug_and_release_target

linux-g++ | linux-g++-64 | linux-g++-32 {
    QMAKE_LFLAGS += -Wl,--rpath=./Local/converter,--rpath=./
    QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

    #for centos6
	#QMAKE_LFLAGS += -Wl,--dynamic-linker=./ld-linux-x86-64.so.2
}

win32:contains(QMAKE_TARGET.arch, x86_64):{
    PLATFORM_BUILD = win_64
    message(windows64)
}
win32:!contains(QMAKE_TARGET.arch, x86_64):{
    PLATFORM_BUILD = win_32
    message(windows32)
}
linux-g++:contains(QMAKE_HOST.arch, x86_64):{
    PLATFORM_BUILD = linux_64
    message(linux64)
}
linux-g++:!contains(QMAKE_HOST.arch, x86_64):{
    PLATFORM_BUILD = linux_32
    message(linux32)
}

mac {
    PLATFORM_BUILD = mac_64
    message(mac)
}

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/build/$$PLATFORM_BUILD/Debug
    message(debug)
} else {
    DESTDIR = $$PWD/build/$$PLATFORM_BUILD/Release
    message(release)
}

PLATFORM_BUILD2 = $$PLATFORM_BUILD
win32 {
CONFIG(debug, debug|release) {
    PLATFORM_BUILD2 = $$PLATFORM_BUILD/DEBUG
}
}

include($$PWD/AscDocumentEditor.pri)


############### converter libs ###############
DESTINATION_SDK_PATH = $$PWD/../../../../../core/build/lib/$$PLATFORM_BUILD2
LIBS += -L$$DESTINATION_SDK_PATH -lascdocumentscore -lPdfReader -lPdfWriter -lDjVuFile -lXpsFile -lHtmlRenderer
##############################################

SOURCES += \
    $$PWD/main_cp.cpp

win32 {
    LIBS += -L$$PWD/../../../../../core/build/cef/$$PLATFORM_BUILD -llibcef
}

linux {
    LIBS += -L$$PWD/../../../../../core/build/cef/$$PLATFORM_BUILD -lcef
}

mac {

DEFINES += _MAC

PROJECT_PATH = $$PWD
#PROJECT_PATH = ..

QMAKE_LFLAGS += -F$${PROJECT_PATH}/../../cefbuilds/mac
LIBS += -framework "Chromium Embedded Framework"

BUNDLE_TARGET = $${DESTDIR}/$${TARGET}.app

QMAKE_POST_LINK += mkdir -p $${BUNDLE_TARGET}/Contents/Frameworks;
QMAKE_POST_LINK += cp -R $${PROJECT_PATH}/../../cefbuilds/mac/ONLYOFFICE\ Helper.app $${BUNDLE_TARGET}/Contents/Frameworks;
QMAKE_POST_LINK += cp -R $${PROJECT_PATH}/../../cefbuilds/mac/Chromium\ Embedded\ Framework.framework $${BUNDLE_TARGET}/Contents/Frameworks;
QMAKE_POST_LINK += cp -R $${PROJECT_PATH}/../../corebuilds/$${PLATFORM_BUILD2}/libascdocumentscore.dylib $${BUNDLE_TARGET}/Contents/Frameworks;

QMAKE_POST_LINK += install_name_tool \
-change \
@executable_path/Chromium\ Embedded\ Framework \
@executable_path/../Frameworks/Chromium\ Embedded\ Framework.framework/Chromium\ Embedded\ Framework \
$${BUNDLE_TARGET}/Contents/MacOS/ONLYOFFICE;

QMAKE_POST_LINK += install_name_tool \
-change \
libascdocumentscore.dylib \
@executable_path/../Frameworks/libascdocumentscore.dylib \
$${BUNDLE_TARGET}/Contents/MacOS/ONLYOFFICE;

QMAKE_POST_LINK += install_name_tool \
-change \
@executable_path/Chromium\ Embedded\ Framework \
@executable_path/../Frameworks/Chromium\ Embedded\ Framework.framework/Chromium\ Embedded\ Framework \
$${BUNDLE_TARGET}/Contents/Frameworks/libascdocumentscore.dylib

}
