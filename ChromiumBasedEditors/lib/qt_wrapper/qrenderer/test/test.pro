CORE_ROOT_DIR = $$PWD/../../../../../../core
PWD_ROOT_DIR = $$PWD

include($$CORE_ROOT_DIR/Common/base.pri)

DESTDIR = $$PWD/build
TEMPLATE = app
TARGET = test

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


ADD_DEPENDENCY(kernel, graphics)

# пока добавил, чтобы иметь возможность не переключать бранч core
DEFINES += "c_nQRenderer=0x0012"

# в тестовом примере никакого импорта
DEFINES += DESKTOP_NO_USE_DYNAMIC_LIBRARY

SOURCES += \
    main.cpp
