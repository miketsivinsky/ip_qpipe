#include <cstdlib>

#include <QThread>

#include "../common/RawStreamTester.h"
#include "ip_qpipe_lib.h"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    unsigned long sTime = 0;
    if(argc > 1) {
        sTime = atol(argv[1]);
    }

    //---
    IP_QPIPE_LIB::TStatus status1 = IP_QPIPE_LIB::createPipeViewRx("slon");
    printf("status1: %1d\n",status1);

    QThread::sleep(sTime);
    return 0;
}
