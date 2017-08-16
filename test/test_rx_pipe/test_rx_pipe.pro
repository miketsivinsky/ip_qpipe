TEMPLATE  = app
TARGET    = test_rx_pipe

CONFIG   += console
CONFIG   -= app_bundle
QT       += core
QT       -= gui

PRJ_DIR   = ../..

include($${PRJ_DIR}/build.pri)

HEADERS += \
           ../common/RawStreamTester.h

SOURCES += \
	     main.cpp \
             ../common/RawStreamTester.cpp

LIBS += -lip_qpipe

