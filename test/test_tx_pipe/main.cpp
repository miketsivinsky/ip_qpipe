#include <cstdlib>

#include "../common/RawStreamTester.h"
#include "ip_qpipe_lib.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main() {
    //---
    printf("[INFO] test_tx_pipe\n");

    //---
    IP_QPIPE_LIB::testTx("slon",1024,32);
    return 0;
}

