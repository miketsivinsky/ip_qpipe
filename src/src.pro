TEMPLATE    = lib
TARGET      = ip_qpipe

CONFIG     += staticlib

PRJ_DIR      = ..

include($${PRJ_DIR}/build.pri)

HEADERS    += \
               $${INC_DIR}/ip_qpipe_lib.h  \
               ip_qpipe.h
SOURCES    += \
               ip_qpipe.cpp   \                
               ip_qpipe_lib.cpp

QT         += testlib

DEFINES += ENA_FW_QT

win32 {
    DEFINES += ENA_WIN_API
}
