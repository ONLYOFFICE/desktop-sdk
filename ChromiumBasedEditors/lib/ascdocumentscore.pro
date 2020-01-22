QT -= core
QT -= gui

TARGET = ascdocumentscore
TEMPLATE = lib

CONFIG += shared
CONFIG += plugin

#CONFIG += build_xp

CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD
include(../../../core/Common/base.pri)

DEFINES += \
    PDFREADER_USE_DYNAMIC_LIBRARY \
    DJVU_USE_DYNAMIC_LIBRARY \
    XPS_USE_DYNAMIC_LIBRARY \
    HTMLRENDERER_USE_DYNAMIC_LIBRARY

ADD_DEPENDENCY(graphics, kernel, UnicodeConverter, PdfWriter, PdfReader, XpsFile, DjVuFile, HtmlRenderer, hunspell, ooxmlsignature)
!core_windows:DEFINES += DOCUMENTSCORE_OPENSSL_SUPPORT

CEF_PROJECT_PRI=$$PWD/cef_pri
build_xp:CEF_PROJECT_PRI=$$PWD/xp
include($$CEF_PROJECT_PRI/cef_base.pri)
include($$CEF_PROJECT_PRI/cef_client.pri)
# ------------------------------------------------------

# for cloud debug
#DEFINES += DEBUG_CLOUD_5_2
#DEFINES += DEBUG_LOCAL_SERVER

####################  BOOST  ###########################

CONFIG += core_boost_libs
include(../../../core/Common/3dParty/boost/boost.pri)

########################################################

HEADERS += \
    ./src/cookiesworker.h \
    ./src/cefwrapper/client_app.h \
    ./src/cefwrapper/client_renderer.h \
    ./src/cefwrapper/client_renderer_params.h \
    ./src/cefwrapper/client_scheme.h \
    ./src/fileconverter.h \
    ./src/fileprinter.h

SOURCES += \
    ./src/cefwrapper/client_scheme_wrapper.cpp \
    ./src/cefwrapper/client_renderer_wrapper.cpp

HEADERS += \
    ./include/base.h \
    ./include/applicationmanager.h \
    ./include/keyboardchecker.h \
    ./include/spellchecker.h \
    ./include/cefapplication.h \
    ./include/cefview.h \
    ./include/applicationmanager_events.h \
    ./src/applicationmanager_p.h \
    ./src/nativeviewer.h \
    ./src/plugins.h \
    ./src/crypto_mode.h \
    ./include/keychain.h \
    ./src/utils.h

SOURCES += \
    ./src/applicationmanager.cpp \
    ./src/keyboardchecker.cpp \
    ./src/spellchecker.cpp \
    ./src/cefapplication.cpp \
    ./src/cefview.cpp \
    ./src/fileprinter.cpp \
    ./src/crypto_mode.cpp \
    ./src/keychain.cpp

SOURCES += \
    ./../../../core/Common/OfficeFileFormatChecker2.cpp \
    ./../../../core/Common/3dParty/pole/pole.cpp \
    ./../../../core/Common/DocxFormat/Source/Base/unicode_util.cpp \
    ./../../../core/HtmlRenderer/src/ASCSVGWriter.cpp \
    ./../../../core/DesktopEditor/fontengine/ApplicationFontsWorker.cpp

SOURCES += \
    ./src/additional/create.cpp

core_windows {

SOURCES += \
    ./src/cefwrapper/monitor_info.cpp

}
