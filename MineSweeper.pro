#-------------------------------------------------
#
# Project created by QtCreator 2016-02-15T12:16:51
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MineSweeper
TEMPLATE = app


SOURCES += main.cpp \
    minesweeper.cpp \
    arena.cpp

HEADERS  += minesweeper.h \
    arena.h

FORMS    += minesweeper.ui

