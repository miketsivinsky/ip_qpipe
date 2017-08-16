#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

#---
CONFIG  -= debug_and_release debug_and_release_target

#---
PRJ_DIR = $${PWD}

#---
if(equals(NO_DEBUG_INFO,1)) {
	DEFINES  += QT_NO_DEBUG_OUTPUT
}

#---
if(defined(IP_QPIPE_INC_DIR,var)) {
	INC_DIR = $${IP_QPIPE_INC_DIR}
} else {
	INC_DIR = $${PRJ_DIR}/include
}

#---
if(defined(IP_QPIPE_OUT_DIR,var)) {
	if(equals(TARGET,"ip_qpipe")) {
		OUT_DIR = $${IP_QPIPE_OUT_DIR}
        } else {
		OUT_DIR = $${PRJ_DIR}/bin
        }
	LIB_DIR = $${IP_QPIPE_OUT_DIR}
} else {
	OUT_DIR = $${PRJ_DIR}/bin
	LIB_DIR = $${PRJ_DIR}/bin
}

#---
if(defined(IP_QPIPE_BLD_DIR,var)) {
	BLD_DIR = $${IP_QPIPE_BLD_DIR}
} else {
	BLD_DIR = $${PRJ_DIR}/build
}

#---
INCLUDEPATH += .                 \
	       $${PRJ_DIR}/src   \
               $${INC_DIR}

#---
CONFIG(release, debug|release) {
   DESTDIR     = $${OUT_DIR}/release
   OBJECTS_DIR = $${BLD_DIR}/$${TARGET}/release
   LIBS       += -L$${LIB_DIR}/release
} else {
   DESTDIR     = $${OUT_DIR}/debug
   OBJECTS_DIR = $${BLD_DIR}/$${TARGET}/debug
   LIBS       += -L$${LIB_DIR}/debug
}

MOC_DIR  = $${OBJECTS_DIR}/moc
