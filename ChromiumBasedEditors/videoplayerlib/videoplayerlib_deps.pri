#CONFIG += libvlc

building_videoplayer_lib {
	libvlc {
		include($$CORE_ROOT_DIR/Common/3dParty/libvlc/libvlc.pri)
	}
} else {
	ADD_DEPENDENCY(videoplayer)

	libvlc {
		LIBS += -L$$CORE_ROOT_DIR/Common/3dParty/libvlc/build/$$CORE_BUILDS_PLATFORM_PREFIX/lib
		core_linux: LIBS += -lvlc -lvlccore
	}

	# if you build with libvlc option - do not forget remove
	# videoplayer library from base build directory (without mediaplayer prefix)
}

libvlc {
    DEFINES += USE_VLC_LIBRARY
} else {
    QT += multimedia multimediawidgets
}
