QT       -= core
QT       -= gui

TARGET   = HtmlFileInternal
CONFIG   += app_bundle
TEMPLATE = app

CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD

CONFIG += core_static_link_libstd
include($$CORE_ROOT_DIR/Common/base.pri)
include($$CORE_ROOT_DIR/Common/3dParty/icu/icu.pri)

DEFINES += ASC_HIDE_WINDOW

CEF_PROJECT_PRI=$$PWD_ROOT_DIR/../../ChromiumBasedEditors/lib/cef_pri
build_xp {
    include($$CEF_PROJECT_PRI/cef_base_xp.pri)
    include($$CEF_PROJECT_PRI/cef_client_xp.pri)
} else {
    include($$CEF_PROJECT_PRI/cef_base.pri)
    include($$CEF_PROJECT_PRI/cef_client.pri)
}

ADD_DEPENDENCY(graphics, kernel, UnicodeConverter)

DESTDIR=$$CORE_BUILDS_LIBRARIES_PATH

HEADERS += \
    $$PWD/src/clienthandler.h \
    $$PWD/src/client_app.h

SOURCES += \
    $$PWD/src/client_renderer_wrapper.cpp

SOURCES += \
    $$PWD/src/main.cpp

#DEFINES += DEBUG_WINDOW_SHOW
#DESTDIR=$$CORE_ROOT_DIR/HtmlFile/test/Debug/HtmlFileInternal
