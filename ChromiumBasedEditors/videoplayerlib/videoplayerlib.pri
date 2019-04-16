QT       += core gui printsupport widgets multimedia multimediawidgets

TARGET = videoplayer
TEMPLATE = lib

CONFIG += shared
CONFIG += plugin

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += BUILD_VIDEO_LIBRARY

CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD
include($$CORE_ROOT_DIR/Common/base.pri)

core_linux {
    DEFINES += QFILEDIALOG_DONTUSENATIVEDIALOG
}

LIBS += -L$$CORE_BUILDS_LIBRARIES_PATH -lkernel

HEADERS += \
    ./src/lib_base.h \
    ./src/qpushbutton_icons.h \
    ./src/qfooterpanel.h \    
    ./src/qvideoslider.h \
    ./src/qvideoplaylist.h \
    ./src/qascvideowidget.h

SOURCES += \
    ./src/lib_base.cpp \
    ./src/qpushbutton_icons.cpp \
    ./src/qfooterpanel.cpp \
    ./src/qascvideoview.cpp \
    ./src/qvideoslider.cpp \
    ./src/qvideoplaylist.cpp \
    ./src/qascvideowidget.cpp

HEADERS += \
    ./qascvideoview.h \

RESOURCES += \
    $$PWD_ROOT_DIR/IconsVideoPlayerLib.qrc

libvlc {

	DEFINES += USE_VLC_LIBRARY

	isEmpty(VLC_DIR) {
		VLC_DIR=$$PWD_ROOT_DIR/vlc
	}

    INCLUDEPATH += $$VLC_DIR/$$CORE_BUILDS_PLATFORM_PREFIX/include

    VLC_LIB_PREFIX=
    core_windows {
        core_debug {
            VLC_LIB_PREFIX=d
        }
    }

    core_windows {
        LIBS += -L$$VLC_DIR/$$CORE_BUILDS_PLATFORM_PREFIX/lib -lVLCQtCore$$VLC_POSTFIX_LIB -lVLCQtWidgets$$VLC_POSTFIX_LIB
        LIBS += -L$$VLC_DIR/$$CORE_BUILDS_PLATFORM_PREFIX/bin
    } else {
        LIBS += -L$$VLC_DIR/$$CORE_BUILDS_PLATFORM_PREFIX/bin -lVLCQtCore$$VLC_POSTFIX_LIB -lVLCQtWidgets$$VLC_POSTFIX_LIB
        LIBS += -L$$VLC_DIR/$$CORE_BUILDS_PLATFORM_PREFIX/bin -lvlc -lvlccore
    }

}
