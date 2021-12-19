CORE_ROOT_DIR = $$PWD/../../../../../../core
PWD_ROOT_DIR = $$PWD

TEMPLATE = app
TARGET = test
CONFIG -= app_bundle

include($$CORE_ROOT_DIR/Common/base.pri)

DESTDIR = $$PWD/build

QT += printsupport widgets gui core

HEADERS += \
    $$PWD/../include/qrenderer.h \
    $$PWD/../include/qprintercontext.h

HEADERS += \
    $$PWD/../src/painting_conversions.h

SOURCES += \
    $$PWD/../src/painting_conversions.cpp \
    $$PWD/../src/qrenderer.cpp \
    $$PWD/../src/qprintercontext.cpp


ADD_DEPENDENCY(UnicodeConverter, kernel, graphics)

CONFIG += pdf_xps_djvu
pdf_xps_djvu {
    DEFINES += SUPPORT_DRAWING_FILE
    ADD_DEPENDENCY(PdfReader, DjVuFile, XpsFile, PdfWriter)
}

core_linux:include($$CORE_ROOT_DIR/Common/3dParty/icu/icu.pri)
core_windows:LIBS += -lgdi32 -ladvapi32 -luser32 -lshell32
core_linux:LIBS += -lz

# пока добавил, чтобы иметь возможность не переключать бранч core
DEFINES += "c_nQRenderer=0x0012"

# в тестовом примере никакого импорта
DEFINES += DESKTOP_NO_USE_DYNAMIC_LIBRARY

SOURCES += \
    main.cpp
