QT -= core
QT -= gui

TARGET = ascdocumentscore
TEMPLATE = lib

CONFIG += shared
CONFIG += plugin

#CONFIG += build_xp

!disable_protection:DEFINES += FEATURE_ENABLE_PROTECT
!disable_signatures:DEFINES += FEATURE_ENABLE_SIGNATURE

CORE_ROOT_DIR = $$PWD/../../../core
include($$CORE_ROOT_DIR/Common/base.pri)

DEFINES += \
    PDFFILE_USE_DYNAMIC_LIBRARY \
    DJVU_USE_DYNAMIC_LIBRARY \
	XPS_USE_DYNAMIC_LIBRARY

DEFINES += DESKTOP_USE_DYNAMIC_LIBRARY_BUILDING

#DEFINES += DISABLE_VSDX

core_mac:DEFINES += _XCODE
!core_windows:DEFINES += DOCUMENTSCORE_OPENSSL_SUPPORT

CEF_PROJECT_PRI=$$PWD/cef_pri
build_xp {
    include($$CEF_PROJECT_PRI/cef_base_xp.pri)
    include($$CEF_PROJECT_PRI/cef_client_xp.pri)
    DESTDIR=$$DESTDIR/xp
} else {
    DEFINES += ENABLE_CEF_EXTENSIONS
    DEFINES += CEF_VERSION_ABOVE_86
    DEFINES += CEF_VERSION_ABOVE_102
    DEFINES += "OVERRIDE=override"

    CONFIG += c++17
    core_windows {
        QMAKE_CXXFLAGS += /std:c++17
    }

    core_linux {
        CONFIG += c++1z
        build_gcc_less_6:INCLUDEPATH += $$PWD/src/polyfill
        cef_version_107 {
            DEFINES += CEF_VERSION_107
            CEF_PROJECT_PRI=$$PWD/cef_pri_107
        }
    }

    core_mac {
        use_v8 {
            DEFINES += CEF_VERSION_103
            CEF_PROJECT_PRI=$$PWD/cef_pri_103
        }
    }


    !cef_version_107 {
	    # DEFINES += CEF_VERSION_138
	}
    include($$CEF_PROJECT_PRI/cef_base.pri)
    include($$CEF_PROJECT_PRI/cef_client.pri)
}

# ------------------------------------------------------

# for cloud debug
#DEFINES += DEBUG_LOCAL_SERVER

####################  BOOST  ###########################

CONFIG += core_boost_libs
CONFIG += core_boost_regex
include($$CORE_ROOT_DIR/Common/3dParty/boost/boost.pri)

########################################################

HEADERS += \
    $$PWD/src/cookiesworker.h \
    $$PWD/src/cefwrapper/client_app.h \
    $$PWD/src/cefwrapper/client_renderer.h \
    $$PWD/src/cefwrapper/client_renderer_params.h \
    $$PWD/src/cefwrapper/client_scheme.h \
    $$PWD/src/fileconverter.h \
	$$PWD/src/x2t.h \
	$$PWD/src/templatesmanager.h \
    $$PWD/src/cefwrapper/client_resource_handler_async.h

SOURCES += \
    $$PWD/src/cefwrapper/client_scheme_wrapper.cpp \
    $$PWD/src/cefwrapper/client_renderer_wrapper.cpp \
    $$PWD/src/cefwrapper/client_resource_handler_async.cpp

HEADERS += \
    $$PWD/include/base.h \
    $$PWD/include/applicationmanager.h \
    $$PWD/include/keyboardchecker.h \
    $$PWD/include/spellchecker.h \
    $$PWD/include/cefapplication.h \
    $$PWD/include/cefview.h \
    $$PWD/include/applicationmanager_events.h \
    $$PWD/src/applicationmanager_p.h \
    $$PWD/src/filelocker.h \
    $$PWD/src/nativeviewer.h \
    $$PWD/src/plugins.h \
    $$PWD/src/providers.h \
    $$PWD/src/cloud_crypto.h \
    $$PWD/src/crypto_mode.h \
    $$PWD/include/keychain.h \
    $$PWD/src/utils.h

SOURCES += \
    $$PWD/src/applicationmanager.cpp \
    $$PWD/src/keyboardchecker.cpp \
    $$PWD/src/spellchecker.cpp \
    $$PWD/src/cefapplication.cpp \
    $$PWD/src/cefview.cpp \
    $$PWD/src/fileprinter.cpp \
    $$PWD/src/crypto_mode.cpp \
    $$PWD/src/keychain.cpp \
    $$PWD/src/filelocker.cpp

SOURCES += \
    $$CORE_ROOT_DIR/Common/OfficeFileFormatChecker2.cpp \
    $$CORE_ROOT_DIR/Common/3dParty/pole/pole.cpp \
	$$CORE_ROOT_DIR/OOXML/Base/unicode_util.cpp

# crypto ----------------------------------
LIBS += -L$$CORE_BUILDS_LIBRARIES_PATH -lCryptoPPLib
LIBS += -L$$CORE_BUILDS_LIBRARIES_PATH -lCompoundFileLib

DEFINES += CRYPTOPP_DISABLE_ASM
SOURCES += \
	$$CORE_ROOT_DIR/MsBinaryFile/DocFile/MemoryStream.cpp \
	$$CORE_ROOT_DIR/MsBinaryFile/XlsFile/Format/Logging/Log.cpp \
	$$CORE_ROOT_DIR/MsBinaryFile/XlsFile/Format/Logging/Logger.cpp
# -----------------------------------------


core_windows {

SOURCES += \
    $$PWD/src/cefwrapper/monitor_info.cpp

}

!core_mac {
    include($$CORE_ROOT_DIR/Common/3dParty/icu/icu.pri)

    HEADERS += \
        $$PWD/src/keyboardlayout.h

    SOURCES += \
        $$PWD/src/keyboardlayout.cpp
}

core_mac {
    LIBS += -framework Security

    HEADERS += ./include/mac_cefview.h

    OBJECTIVE_SOURCES += \
        $$PWD/src/widget_impl.mm \
        $$PWD/src/mac_application.mm

    use_v8:DEFINES += OLD_MACOS_SYSTEM

	DEFINES += NO_SUPPORT_MEDIA_PLAYER
}

core_linux {
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"
    QMAKE_LFLAGS += -Wl,--disable-new-dtags

    LIBS += -lX11 -lX11-xcb -lxkbcommon-x11 -lxkbcommon
}

ADD_DEPENDENCY(graphics, kernel, UnicodeConverter, kernel_network, PdfFile, XpsFile, DjVuFile, hunspell, ooxmlsignature)
