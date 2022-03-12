######################################################################
# Automatically generated by qmake (3.1) Sun Sep 15 06:41:44 2019
#
# And modified by DJ to make it work: Added the two QT += lines.
#
######################################################################

QMAKE_CFLAGS += -fdiagnostics-color=auto
QMAKE_CXXFLAGS += -fdiagnostics-color=auto

QT += core gui
QT += widgets
QT += network

TEMPLATE = app
TARGET = gui
INCLUDEPATH += .

INCLUDEPATH += /usr/include/qt5/QtWidgets /usr/include/qt5/qwt/

LIBS += -lpthread -lm -lmirsdrapi-rsp -lfftw3
LIBS += -lluaradio -lluajit-5.1 -Wl,-E
LIBS += -lliquid -lasound 
#LIBS += -L"C:\qwt2\qwt-6.1.4\lib"-lqwt

QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter

#DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += MainWindow.h ScopePlotter.h cfreqctrl.h meter.h Rxr.h
FORMS += QtBase-001.ui
SOURCES += Main.cpp MainWindow.cpp ScopePlotter.cpp adpcm.c  cfreqctrl.cpp meter.cpp Rxr.cpp
