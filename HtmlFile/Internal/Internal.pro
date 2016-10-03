QT       -= core
QT       -= gui

TARGET   = HtmlFileInternal
CONFIG   += app_bundle
TEMPLATE = app

DEFINES += ASC_HIDE_WINDOW

CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD
include($$CORE_ROOT_DIR/Common/base.pri)

win32 {
    include(./Internal_windows.pri)
}
linux-g++ | linux-g++-64 | linux-g++-32 {
    include(./Internal_linux.pri)
}
