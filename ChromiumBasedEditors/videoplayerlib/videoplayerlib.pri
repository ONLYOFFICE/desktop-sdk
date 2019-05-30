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
    $$PWD/src/lib_base.h \
    $$PWD/src/qpushbutton_icons.h \
    $$PWD/src/qfooterpanel.h \
    $$PWD/src/qvideoslider.h \
    $$PWD/src/qvideoplaylist.h \
    $$PWD/src/qascvideowidget.h

SOURCES += \
    $$PWD/src/lib_base.cpp \
    $$PWD/src/qpushbutton_icons.cpp \
    $$PWD/src/qfooterpanel.cpp \
    $$PWD/src/qascvideoview.cpp \
    $$PWD/src/qvideoslider.cpp \
    $$PWD/src/qvideoplaylist.cpp \
    $$PWD/src/qascvideowidget.cpp

HEADERS += \
    $$PWD/qascvideoview.h \

RESOURCES += \
    $$PWD/IconsVideoPlayerLib.qrc

VIDEO_PLAYER_VLC_DIR = $$(VIDEO_PLAYER_VLC_DIR)
!isEmpty(VIDEO_PLAYER_VLC_DIR){
    CONFIG += libvlc
}

libvlc {

	DEFINES += USE_VLC_LIBRARY
	INCLUDEPATH += $$VIDEO_PLAYER_VLC_DIR/$$CORE_BUILDS_PLATFORM_PREFIX/include

    VLC_LIB_PREFIX=
    core_windows {
        core_debug {
            VLC_LIB_PREFIX=d
        }
    }

    core_windows {
        LIBS += -L$$VIDEO_PLAYER_VLC_DIR/$$CORE_BUILDS_PLATFORM_PREFIX/lib -lVLCQtCore$$VLC_POSTFIX_LIB -lVLCQtWidgets$$VLC_POSTFIX_LIB
        LIBS += -L$$VIDEO_PLAYER_VLC_DIR/$$CORE_BUILDS_PLATFORM_PREFIX/bin
    } else {
        LIBS += -L$$VIDEO_PLAYER_VLC_DIR/$$CORE_BUILDS_PLATFORM_PREFIX/bin -lVLCQtCore$$VLC_POSTFIX_LIB -lVLCQtWidgets$$VLC_POSTFIX_LIB
        LIBS += -L$$VIDEO_PLAYER_VLC_DIR/$$CORE_BUILDS_PLATFORM_PREFIX/bin -lvlc -lvlccore
    }

}
