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

    printf("\n");
    printf("----- rx pipe -----\n");
    printf("status:    %6d\n",status);
    printf("chunkNum:  %6d\n",pipeInfo.chunkNum);
    printf("chunkSize: %6d\n",pipeInfo.chunkSize);
    printf("txReady:   %6d\n",pipeInfo.txReady);
    for(auto k = 0; k < pipeInfo.MaxRxNum; ++k) {
        printf("rxReady[%1d]: %5d\n",k,pipeInfo.rxReady[k]);
    }
    printf("\n");

    QThread::sleep(sTime);
    return 0;
}
