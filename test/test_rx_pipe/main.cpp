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
    IP_QPIPE_LIB::TPipeInfo pipeInfo;

    IP_QPIPE_LIB::TStatus status = IP_QPIPE_LIB::createPipeViewRx("slon",&pipeInfo);
    printf("----- rx pipe -----\n");
    printf("status: %1d\n",status);
    printf("chunkNum: %2d, chunkSize: %2d\n",pipeInfo.chunkNum,pipeInfo.chunkSize);
    printf("txReady: %1d\n",pipeInfo.txReady);
    for(auto k = 0; k < pipeInfo.MaxRxNum; ++k) {
        printf("rxReady[%1d]: %1d\n",k,pipeInfo.rxReady[k]);
    }

    QThread::sleep(sTime);
    return 0;
}
