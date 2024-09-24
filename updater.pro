#-------------------------------------------------
#
# Project created by QtCreator 2013-01-07T23:21:33
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT += widgets


TARGET = updater
TEMPLATE = app

CONFIG   -= console


SOURCES += main.cpp\
        window.cpp \
    aupdater.cpp \
    aftp.cpp \
    alocal.cpp \
    afunc.cpp

HEADERS  += window.h \
    aupdater.h \
    aftp.h \
    alocal.h \
    afunc.h

FORMS    += window.ui

DESTDIR += c:/Updater/
