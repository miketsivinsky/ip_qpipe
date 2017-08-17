#include <cstdlib>

#include "../common/RawStreamTester.h"
#include "ip_qpipe_lib.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main() {
    //---
    printf("[INFO] test_tx_pipe\n");

    //---
    IP_QPIPE_LIB::TStatus status1 = IP_QPIPE_LIB::createPipeViewTx("slon",1020,32);
    IP_QPIPE_LIB::TStatus status2 = IP_QPIPE_LIB::createPipeViewTx("slon",1020,32);
    printf("status1: %1d, status2: %1d\n",status1,status2);
    return 0;
}

