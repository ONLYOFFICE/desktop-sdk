QT       += core gui printsupport widgets multimedia multimediawidgets

TARGET = qtascdocumentscore
TEMPLATE = lib

CONFIG += shared
CONFIG += plugin

CORE_ROOT_DIR = $$PWD/../../../../core
PWD_ROOT_DIR = $$PWD
include($$CORE_ROOT_DIR/Common/base.pri)

DEFINES += DESKTOP_USE_DYNAMIC_LIBRARY_BUILDING
ADD_DEPENDENCY(kernel, graphics, ascdocumentscore)

include($$CORE_ROOT_DIR/../desktop-sdk/ChromiumBasedEditors/videoplayerlib/videoplayerlib_deps.pri)

HEADERS += \
    $$PWD/include/qcefview.h \
    $$PWD/include/qcefview_media.h \
    $$PWD/include/qascprinter.h \
	$$PWD/qrenderer/include/qrenderer.h \
    $$PWD/include/qexternalmessageloop.h \
    $$PWD/include/qascapplicationmanager.h \
    $$PWD/include/qdpichecker.h

SOURCES += \
    $$PWD/src/qcefview.cpp \
    $$PWD/src/qcefview_media.cpp \
    $$PWD/src/qascprinter.cpp \
	$$PWD/qrenderer/src/qrenderer.cpp \
    $$PWD/src/qexternalmessageloop.cpp \
    $$PWD/src/qascapplicationmanager.cpp \
    $$PWD/src/qdpichecker.cpp

core_windows {
    LIBS += -luser32
	build_xp:DESTDIR=$$DESTDIR/xp
}

core_linux {
    !greaterThan(QT_MAJOR_VERSION, 5) {
        QT += x11extras
    } else {
        QT += gui-private
    }

    LIBS += -Wl,-unresolved-symbols=ignore-in-shared-libs
    ADD_DEPENDENCY(graphics, kernel, UnicodeConverter)
    LIBS += -lX11

}
