QT       += core gui printsupport widgets multimedia multimediawidgets

TARGET = videoplayer
TEMPLATE = lib

CONFIG += shared
CONFIG += plugin

include($$PWD/videoplayerlib.pri)

PRO_SUFFIX=
libvlc {
    PRO_SUFFIX=mediaplayer
}
build_xp {
    isEmpty(PRO_SUFFIX) {
        PRO_SUFFIX=xp
    } else {
        PRO_SUFFIX=$$PRO_SUFFIX/xp
    }
}

!isEmpty(PRO_SUFFIX) {
    DESTDIR=$$DESTDIR/$$PRO_SUFFIX
    OBJECTS_DIR=$$OBJECTS_DIR/$$PRO_SUFFIX
    MOC_DIR=$$MOC_DIR/$$PRO_SUFFIX
    RCC_DIR=$$RCC_DIR/$$PRO_SUFFIX
    UI_DIR=$$UI_DIR/$$PRO_SUFFIX
}
