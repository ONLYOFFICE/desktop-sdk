#CONFIG += libvlc

building_videoplayer_lib {
	libvlc {
		DEFINES += USE_VLC_LIBRARY
		include($$CORE_ROOT_DIR/Common/3dParty/libvlc/libvlc.pri)
	}
} else {
	ADD_DEPENDENCY(videoplayer)

	# if you build with libvlc option - do not forget remove
	# videoplayer library from base build directory (without mediaplayer prefix)
}
