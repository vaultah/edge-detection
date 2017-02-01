#-------------------------------------------------
#
# Project created by QtCreator 2016-12-29T07:23:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = edge-detection
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    algorithms.cpp

HEADERS  += mainwindow.h \
    kernels.h \
    algorithms.h

FORMS    += mainwindow.ui

CONFIG += c++14
QMAKE_CXXFLAGS += -Wall
