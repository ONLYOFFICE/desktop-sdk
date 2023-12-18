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

	!build_xp {
		CONFIG -= embed_manifest_exe
		OTHER_FILES += $$PWD/helper.exe.manifest
	}
}

core_linux {
	QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
	QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"

	LIBS += -Wl,-unresolved-symbols=ignore-in-shared-libs
	ADD_DEPENDENCY(graphics, kernel, kernel_network, UnicodeConverter)
}

SOURCES += helper_main.cpp
