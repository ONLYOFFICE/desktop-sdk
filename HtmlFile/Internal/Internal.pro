QT       -= core
QT       -= gui

TARGET   = HtmlFileInternal
CONFIG   += app_bundle
TEMPLATE = app

CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD
include($$CORE_ROOT_DIR/Common/base.pri)
include($$CORE_ROOT_DIR/Common/3dParty/icu/icu.pri)
include($$CORE_ROOT_DIR/Common/3dParty/curl/curl.pri)

include($$CORE_ROOT_DIR/../desktop-sdk/ChromiumBasedEditors/lib/AscDocumentsCore_base.pri)
LIBS += -L$$CORE_BUILDS_LIBRARIES_PATH -lUnicodeConverter -lkernel -lgraphics


DESTDIR=$$CORE_BUILDS_LIBRARIES_PATH

HEADERS += \
    $$PWD/src/clienthandler.h \
    $$PWD/src/client_app.h

SOURCES += \
    $$PWD/src/client_renderer_wrapper.cpp

SOURCES += \
    $$PWD/src/main.cpp

win32 {
    include($$CORE_ROOT_DIR/../desktop-sdk/ChromiumBasedEditors/lib/AscDocumentsCore_windows.pri)
}
linux-g++ | linux-g++-64 | linux-g++-32 {
    include($$CORE_ROOT_DIR/../desktop-sdk/ChromiumBasedEditors/lib/AscDocumentsCore_linux.pri)

    CONFIG += link_pkgconfig c++11
    PKGCONFIG += glib-2.0 gdk-2.0 gtkglext-1.0 atk cairo gtk+-unix-print-2.0
    LIBS += -lz

    #CONFIG += build_for_centos6

    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"
    QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

    build_for_centos6 {
		core_linux_64 {
			QMAKE_LFLAGS += -Wl,--dynamic-linker=./ld-linux-x86-64.so.2
		} else {
			QMAKE_LFLAGS += -Wl,--dynamic-linker=./ld-linux.so.2
		}
		DESTDIR = $$DESTDIR/CentOS6
    }
}
