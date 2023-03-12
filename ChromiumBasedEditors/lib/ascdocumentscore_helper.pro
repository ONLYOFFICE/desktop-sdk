QT -= core
QT -= gui

TARGET = editors_helper
TEMPLATE = app
CONFIG += app_bundle

#CONFIG += build_xp

CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD
include(../../../core/Common/base.pri)

DESTDIR = $$CORE_BUILDS_BINARY_PATH

ADD_DEPENDENCY(ascdocumentscore)

core_windows {
    build_xp:DESTDIR=$$DESTDIR/xp
}

core_linux {
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"
    LIBS += -L$$PWD/../../../core/Common/3dParty/cef/$$CORE_BUILDS_PLATFORM_PREFIX/build -lcef
    include($$PWD/../../../core/Common/3dParty/icu/icu.pri)
    ADD_DEPENDENCY(graphics, kernel, kernel_network, UnicodeConverter, PdfFile, XpsFile, DjVuFile, HtmlRenderer, hunspell, ooxmlsignature)
}

SOURCES += helper_main.cpp

