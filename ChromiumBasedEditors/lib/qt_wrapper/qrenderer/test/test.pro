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
	$$PWD/../include/qprintercontext.h \
	$$PWD/../../include/qascprinter.h

SOURCES += \
    $$PWD/../src/qrenderer.cpp \
	$$PWD/../src/qprintercontext.cpp \
	$$PWD/../../src/qascprinter.cpp


ADD_DEPENDENCY(UnicodeConverter, kernel, graphics)

CONFIG += pdf_xps_djvu
pdf_xps_djvu {
    DEFINES += SUPPORT_DRAWING_FILE
    ADD_DEPENDENCY(PdfReader, DjVuFile, XpsFile, PdfWriter)
}

core_linux:include($$CORE_ROOT_DIR/Common/3dParty/icu/icu.pri)
core_windows:LIBS += -lgdi32 -ladvapi32 -luser32 -lshell32
core_linux:LIBS += -lz

# в тестовом примере никакого импорта
DEFINES += DESKTOP_NO_USE_DYNAMIC_LIBRARY

SOURCES += \
    main.cpp
