#include <cstdlib>

#include "../common/RawStreamTester.h"
#include "ip_qpipe_lib.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main() {

    //---
    printf("[INFO] test_tx_pipe: %d\n",IP_QPIPE_LIB::testTx(0,0,0));

    return 0;
}

