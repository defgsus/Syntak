#-------------------------------------------------
#
# Project created by QtCreator 2016-09-25T21:29:07
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = syntak
CONFIG   += c++11 console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    Tokens.cpp \
    Rules.cpp \
    Parser.cpp \
    main.cpp \
    Parser_yabnf.cpp

HEADERS += \
    Tokens.h \
    Rules.h \
    Parser.h \
    error.h

