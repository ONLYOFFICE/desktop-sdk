QT -= core
QT -= gui

TARGET = ascdocumentscore
TEMPLATE = lib

CONFIG += shared
CONFIG += plugin

CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD
include(../../../core/Common/base.pri)

DEFINES += \
    PDFREADER_USE_DYNAMIC_LIBRARY \
    DJVU_USE_DYNAMIC_LIBRARY \
    XPS_USE_DYNAMIC_LIBRARY \
    HTMLRENDERER_USE_DYNAMIC_LIBRARY

LIBS += -L$$CORE_BUILDS_LIBRARIES_PATH -lOfficeUtils -lgraphics -llibxml -lhunspell
LIBS += -L$$CORE_BUILDS_LIBRARIES_PATH -lPdfReader -lPdfWriter -lDjVuFile -lXpsFile -lHtmlRenderer -lUnicodeConverter -looxmlsignature
# ------------------------------------------------------

####################  BOOST  ###########################

CONFIG += core_boost_libs
include(../../../core/Common/3dParty/boost/boost.pri)

########################################################
#CONFIG += build_xp

!build_xp {
    include($$PWD/AscDocumentsCore_base.pri)
    include($$PWD/AscDocumentsCore_windows.pri)
} else {
    DEFINES += CEF_2623
    DESTDIR=$$DESTDIR/xp
    include($$PWD/AscDocumentsCore_base_xp.pri)
}

HEADERS += \
    ./src/cookiesworker.h \
    ./src/cefwrapper/client_app.h \
    ./src/cefwrapper/client_renderer.h \
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
    ./src/crypto_mode.h

SOURCES += \
    ./src/applicationmanager.cpp \
    ./src/keyboardchecker.cpp \
    ./src/spellchecker.cpp \
    ./src/cefapplication.cpp \
    ./src/cefview.cpp \
    ./src/fileprinter.cpp \
    ./src/crypto_mode.cpp

SOURCES += \
    ./../../../core/Common/OfficeFileFormatChecker2.cpp \
    ./../../../core/Common/3dParty/pole/pole.cpp \
    ./../../../core/Common/DocxFormat/Source/Base/unicode_util.cpp \
    ./../../../core/HtmlRenderer/src/ASCSVGWriter.cpp \
    ./../../../core/Common/FileDownloader/FileDownloader_win.cpp

SOURCES += \
    ./src/additional/create.cpp

SOURCES += \
    ./src/cefwrapper/monitor_info.cpp
