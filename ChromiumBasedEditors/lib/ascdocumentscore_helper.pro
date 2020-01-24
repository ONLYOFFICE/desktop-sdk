QT -= core
QT -= gui

TARGET = ascdocumentscore_helper
TEMPLATE = app
CONFIG += app_bundle

#CONFIG += build_xp

CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD
include(../../../core/Common/base.pri)

DESTDIR = $$CORE_BUILDS_BINARY_PATH

ADD_DEPENDENCY(graphics, kernel, UnicodeConverter, PdfWriter, PdfReader, XpsFile, DjVuFile, HtmlRenderer, hunspell, ooxmlsignature, ascdocumentscore)

SOURCES += helper_main.cpp
