#-------------------------------------------------
#
# Project created by QtCreator 2016-09-25T21:29:07
#
#-------------------------------------------------

QT       += core testlib
QT       -= gui

TARGET = syntak
CONFIG   += c++11 console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../syntak

SOURCES += \
    ../../syntak/Tokens.cpp \
    ../../syntak/Rules.cpp \
    ../../syntak/Parser.cpp \
    main.cpp 

HEADERS += \
    ../../syntak/Tokens.h \
    ../../syntak/Rules.h \
    ../../syntak/Parser.h \
    MathParser.h

