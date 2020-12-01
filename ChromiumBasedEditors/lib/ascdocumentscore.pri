QT -= core
QT -= gui

TARGET = ascdocumentscore
TEMPLATE = lib

CONFIG += shared
CONFIG += plugin

#CONFIG += build_xp

CORE_ROOT_DIR = $$PWD/../../../core
include($$CORE_ROOT_DIR/Common/base.pri)

DEFINES += \
    PDFREADER_USE_DYNAMIC_LIBRARY \
    DJVU_USE_DYNAMIC_LIBRARY \
    XPS_USE_DYNAMIC_LIBRARY \
    HTMLRENDERER_USE_DYNAMIC_LIBRARY

DEFINES += DESKTOP_USE_DYNAMIC_LIBRARY_BUILDING

core_mac:DEFINES += _XCODE

ADD_DEPENDENCY(graphics, kernel, UnicodeConverter, PdfWriter, PdfReader, XpsFile, DjVuFile, HtmlRenderer, hunspell, ooxmlsignature)
!core_windows:DEFINES += DOCUMENTSCORE_OPENSSL_SUPPORT

CEF_PROJECT_PRI=$$PWD/cef_pri
build_xp {
    include($$CEF_PROJECT_PRI/cef_base_xp.pri)
    include($$CEF_PROJECT_PRI/cef_client_xp.pri)
    DESTDIR=$$DESTDIR/xp
} else {
    !core_linux {
        DEFINES += CEF_VERSION_ABOVE_86
        CEF_PROJECT_PRI=$$PWD/cef_pri_87
    }
    include($$CEF_PROJECT_PRI/cef_base.pri)
    include($$CEF_PROJECT_PRI/cef_client.pri)
}

# ------------------------------------------------------

# for cloud debug
#DEFINES += DEBUG_LOCAL_SERVER

####################  BOOST  ###########################

CONFIG += core_boost_libs
include($$CORE_ROOT_DIR/Common/3dParty/boost/boost.pri)

########################################################

HEADERS += \
    $$PWD/src/cookiesworker.h \
    $$PWD/src/cefwrapper/client_app.h \
    $$PWD/src/cefwrapper/client_renderer.h \
    $$PWD/src/cefwrapper/client_renderer_params.h \
    $$PWD/src/cefwrapper/client_scheme.h \
    $$PWD/src/fileconverter.h \
    $$PWD/src/fileprinter.h

SOURCES += \
    $$PWD/src/cefwrapper/client_scheme_wrapper.cpp \
    $$PWD/src/cefwrapper/client_renderer_wrapper.cpp

HEADERS += \
    $$PWD/include/base.h \
    $$PWD/include/applicationmanager.h \
    $$PWD/include/keyboardchecker.h \
    $$PWD/include/spellchecker.h \
    $$PWD/include/cefapplication.h \
    $$PWD/include/cefview.h \
    $$PWD/include/applicationmanager_events.h \
    $$PWD/src/applicationmanager_p.h \
    $$PWD/src/nativeviewer.h \
    $$PWD/src/plugins.h \
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
    $$PWD/src/keychain.cpp

SOURCES += \
    $$CORE_ROOT_DIR/Common/OfficeFileFormatChecker2.cpp \
    $$CORE_ROOT_DIR/Common/3dParty/pole/pole.cpp \
    $$CORE_ROOT_DIR/Common/DocxFormat/Source/Base/unicode_util.cpp \
    $$CORE_ROOT_DIR/HtmlRenderer/src/ASCSVGWriter.cpp \
    $$CORE_ROOT_DIR/DesktopEditor/fontengine/ApplicationFontsWorker.cpp

# crypto ----------------------------------
LIBS += -L$$CORE_BUILDS_LIBRARIES_PATH -lCryptoPPLib
DEFINES += CRYPTOPP_DISABLE_ASM
HEADERS += \
    $$CORE_ROOT_DIR/OfficeCryptReader/source/ECMACryptFile.h \
    $$CORE_ROOT_DIR/OfficeCryptReader/source/CryptTransform.h

SOURCES += \
    $$CORE_ROOT_DIR/OfficeCryptReader/source/ECMACryptFile.cpp \
    $$CORE_ROOT_DIR/OfficeCryptReader/source/CryptTransform.cpp
# -----------------------------------------


core_windows {

SOURCES += \
    $$PWD/src/cefwrapper/monitor_info.cpp

}

core_mac {
    LIBS += -framework Security

    HEADERS += ./include/mac_cefview.h

    OBJECTIVE_SOURCES += \
        $$PWD/src/widget_impl.mm \
        $$PWD/src/mac_application.mm
}

core_linux {
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"
    QMAKE_LFLAGS += -Wl,--disable-new-dtags
}
