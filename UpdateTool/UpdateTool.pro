#-------------------------------------------------
#
# Project created by QtCreator 2014-03-22T17:19:26
#
#-------------------------------------------------

QT       += core gui\
           network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UpdateTool
TEMPLATE = app


SOURCES += main.cpp\
        ./QHexEdit/qhexedit.cpp \
    ./QHexEdit/qhexedit_p.cpp \
    ./QHexEdit/xbytearray.cpp \
    ./QHexEdit/commands.cpp \
        mainwindow.cpp

HEADERS  += mainwindow.h\
        ./QHexEdit/qhexedit.h \
    ./QHexEdit/qhexedit_p.h \
    ./QHexEdit/xbytearray.h \
    ./QHexEdit/commands.h

FORMS    += mainwindow.ui

INCLUDEPATH+=./QHexEdit/
