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
	$$PWD/src/style/style_options.h \
	$$PWD/src/videoplayerlib_export.h \
	$$PWD/src/qiconpushbutton.h \
	$$PWD/src/qfooterpanel_private.h \
	$$PWD/src/qvideoslider.h \
	$$PWD/src/qvideoplaylist.h \
	$$PWD/src/qascvideowidget.h \
	$$PWD/src/qmultimedia.h \
	$$PWD/src/style/skins_footer.h

SOURCES += \
	$$PWD/src/lib_base.cpp \
	$$PWD/src/qiconpushbutton.cpp \
	$$PWD/src/qfooterpanel.cpp \
	$$PWD/src/qascvideoview.cpp \
	$$PWD/src/qvideoslider.cpp \
	$$PWD/src/qvideoplaylist.cpp \
	$$PWD/src/qascvideowidget.cpp \
	$$PWD/src/qwidgetutils.cpp \
	$$PWD/src/style/skins_footer.cpp

HEADERS += \
	$$PWD/qascvideoview.h \
	$$PWD/qfooterpanel.h \
	$$PWD/qwidgetutils.h

RESOURCES += \
	$$PWD/IconsVideoPlayerLib.qrc

CONFIG += building_videoplayer_lib
include(videoplayerlib_deps.pri)

!libvlc {
	HEADERS += $$PWD/src/qascmediaplayer.h
}
