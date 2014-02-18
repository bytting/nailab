#-------------------------------------------------
#
# Project created by QtCreator 2013-02-08T19:52:49
#
#-------------------------------------------------

CONFIG += c++11

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nailab
TEMPLATE = app

SOURCES += main.cpp\    
    nailab.cpp \
    createbeaker.cpp \
    createdetector.cpp \
    dbutils.cpp \    
    mcalib.cpp \
    winutils.cpp \
    createdetectorbeaker.cpp \
    editdetectorbeaker.cpp

HEADERS  += nailab.h \
    createbeaker.h \
    createdetector.h \
    beaker.h \
    detector.h \
    dbutils.h \    
    mcalib.h \
    winutils.h \
    settings.h \
    sampleinput.h \
    createdetectorbeaker.h \
    editdetectorbeaker.h \
    exceptions.h

FORMS    += nailab.ui \
    createbeaker.ui \
    createdetector.ui \
    createdetectorbeaker.ui \
    editdetectorbeaker.ui

RESOURCES += nailab.qrc

LIBS += -lAdvapi32

win32: LIBS += -L$$PWD/../../../GENIE2K/S560/ -lSad

INCLUDEPATH += $$PWD/../../../GENIE2K/S560
DEPENDPATH += $$PWD/../../../GENIE2K/S560

win32: PRE_TARGETDEPS += $$PWD/../../../GENIE2K/S560/Sad.lib

win32: LIBS += -L$$PWD/../../../GENIE2K/S560/ -lUtility

INCLUDEPATH += $$PWD/../../../GENIE2K/S560
DEPENDPATH += $$PWD/../../../GENIE2K/S560

win32: PRE_TARGETDEPS += $$PWD/../../../GENIE2K/S560/Utility.lib
