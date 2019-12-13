QT -= core
QT -= gui

TARGET = ascdocumentscore
TEMPLATE = lib

CONFIG += shared
CONFIG += plugin

CONFIG += core_static_link_libstd
CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD
include(../../../core/Common/base.pri)

QMAKE_CXXFLAGS += -fPIC

QMAKE_LFLAGS += -Wl,--rpath=./

DEFINES += \
    PDFREADER_USE_DYNAMIC_LIBRARY \
    DJVU_USE_DYNAMIC_LIBRARY \
    XPS_USE_DYNAMIC_LIBRARY \
    HTMLRENDERER_USE_DYNAMIC_LIBRARY

ADD_DEPENDENCY(graphics, kernel, UnicodeConverter, PdfWriter, PdfReader, XpsFile, DjVuFile, HtmlRenderer, hunspell, ooxmlsignature)
# ------------------------------------------------------

DEFINES += DOCUMENTSCORE_OPENSSL_SUPPORT

include($$PWD/AscDocumentsCore_base.pri)
include($$PWD/AscDocumentsCore_linux.pri)

####################  BOOST  ###########################

CONFIG += core_boost_libs
CONFIG += core_boost_fpic
include($$CORE_ROOT_DIR/Common/3dParty/boost/boost.pri)

########################################################

CONFIG += link_pkgconfig c++11
PKGCONFIG += glib-2.0 gdk-2.0 atk cairo gtk+-unix-print-2.0
LIBS += -lX11

HEADERS += \
    ./src/cookiesworker.h \
    ./src/cefwrapper/client_app.h \
    ./src/cefwrapper/client_renderer.h \
    ./src/cefwrapper/client_scheme.h \
    ./src/fileconverter.h \
    ./src/fileprinter.h \
    ./src/applicationmanager_p.h \
    ./src/nativeviewer.h \
    ./src/plugins.h \
    ./src/crypto_mode.h \
    ./include/keychain.h

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
    ./include/applicationmanager_events.h

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
