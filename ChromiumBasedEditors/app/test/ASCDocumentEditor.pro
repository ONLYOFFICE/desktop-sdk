QT  += core gui widgets gui-private widgets-private core-private printsupport

TEMPLATE = app
CONFIG += app_bundle

CONFIG -= debug_and_release debug_and_release_target

win32:contains(QMAKE_TARGET.arch, x86_64):{
    PLATFORM_BUILD = win_64
    message(windows64)
}
win32:!contains(QMAKE_TARGET.arch, x86_64):{
    PLATFORM_BUILD = win_32
    message(windows32)
}

include($$PWD/src/AscDocumentEditor.pri)

HEADERS += \
    mainwindow.h \
    qmainpanel.h \
    qwinhost.h \
    qwinwidget.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    qmainpanel.cpp \
    qwinhost.cpp \
    qwinwidget.cpp

LIBS += -L$$PWD/../../../../build/cef/$$PLATFORM_BUILD -llibcef

############### converter libs ###############
DESTINATION_SDK_PATH = $$PWD/../../../../build/lib

# WINDOWS
win32:contains(QMAKE_TARGET.arch, x86_64):{
CONFIG(debug, debug|release) {
    DESTINATION_SDK_PATH = $$DESTINATION_SDK_PATH/win_64/DEBUG
} else {
    DESTINATION_SDK_PATH = $$DESTINATION_SDK_PATH/win_64
}
}
win32:!contains(QMAKE_TARGET.arch, x86_64):{
CONFIG(debug, debug|release) {
    DESTINATION_SDK_PATH = $$DESTINATION_SDK_PATH/win_32/DEBUG
} else {
    DESTINATION_SDK_PATH = $$DESTINATION_SDK_PATH/win_32
}
}

linux-g++:contains(QMAKE_HOST.arch, x86_64):{
    DESTINATION_SDK_PATH = $$DESTINATION_SDK_PATH/linux_64
}
linux-g++:!contains(QMAKE_HOST.arch, x86_64):{
    DESTINATION_SDK_PATH = $$DESTINATION_SDK_PATH/linux_32
}

mac {
    DESTINATION_SDK_PATH = $$DESTINATION_SDK_PATH/mac_64
}
LIBS += -L$$DESTINATION_SDK_PATH -lPdfReader -lPdfWriter -lDjVuFile -lXpsFile -lascdocumentscore
##############################################

contains(QMAKE_TARGET.arch, x86_64):{
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
} else {
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
}
