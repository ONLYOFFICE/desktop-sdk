QT       += core gui printsupport widgets multimedia multimediawidgets

TARGET = videoplayer
TEMPLATE = lib

CONFIG += shared
CONFIG += plugin

include($$PWD/videoplayerlib.pri)
