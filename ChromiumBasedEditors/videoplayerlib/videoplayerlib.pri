DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += BUILD_VIDEO_LIBRARY

CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD
include($$CORE_ROOT_DIR/Common/base.pri)

core_linux {
    DEFINES += QFILEDIALOG_DONTUSENATIVEDIALOG
}

ADD_DEPENDENCY(kernel)

HEADERS += \
    $$PWD/src/lib_base.h \
    $$PWD/src/qpushbutton_icons.h \
    $$PWD/src/qfooterpanel.h \
    $$PWD/src/qvideoslider.h \
    $$PWD/src/qvideoplaylist.h \
    $$PWD/src/qascvideowidget.h \
    $$PWD/src/qascmediaplayer.h

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

#VIDEO_PLAYER_VLC_DIR = $$(VIDEO_PLAYER_VLC_DIR)
VIDEO_PLAYER_VLC_DIR = $$CORE_ROOT_DIR/Common/3dParty/libvlc
!isEmpty(VIDEO_PLAYER_VLC_DIR){
    CONFIG += libvlc
}

libvlc {
    DEFINES += USE_VLC_LIBRARY
    include($$CORE_ROOT_DIR/Common/3dParty/libvlc/libvlc.pri)
}
