#-------------------------------------------------
#
# Project created by QtCreator 2018-01-29T20:46:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myNote
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mychild.cpp \
    linenumberarea.cpp \
    error.cpp \
    debug.cpp

HEADERS  += mainwindow.h \
    mychild.h \
    linenumberarea.h \
    error.h \
    debug.h

RC_FILE = proj.rc
