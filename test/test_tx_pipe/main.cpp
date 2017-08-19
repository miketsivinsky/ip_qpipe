#include <cstdlib>

#include <QThread>

#include "../common/RawStreamTester.h"
#include "ip_qpipe_lib.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main() {
    printf("[INFO] test_tx_pipe\n");

    //---
    IP_QPIPE_LIB::TStatus status1 = IP_QPIPE_LIB::createPipeViewTx("slon",1024,32);
    printf("status1: %1d\n",status1);

    QThread::sleep(1);
    return 0;
}

