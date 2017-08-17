#include <cstdlib>

#include "../common/RawStreamTester.h"
#include "ip_qpipe_lib.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main() {
    //---
    printf("[INFO] test_tx_pipe\n");

    //---
    bool status = IP_QPIPE_LIB::testTx("slon",1020,32);
    printf("status: %d\n",status);
    return 0;
}

