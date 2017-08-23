#include <cstdlib>

#include <QThread>

#include "../common/RawStreamTester.h"
#include "ip_qpipe_lib.h"


//------------------------------------------------------------------------------
void printPipeRxInfo(IP_QPIPE_LIB::TStatus status, const IP_QPIPE_LIB::TPipeRxParams& params);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    unsigned long sTime = 0;
    if(argc > 1) {
        sTime = atol(argv[1]);
    }

    //---
    //---
    IP_QPIPE_LIB::TPipeRxParams rxParams;
    rxParams.pipeKey            = "slon";

    IP_QPIPE_LIB::TStatus status = IP_QPIPE_LIB::createPipeViewRx(rxParams);
    printPipeRxInfo(status,rxParams);

    QThread::sleep(sTime);
    return 0;
}

//------------------------------------------------------------------------------
void printPipeRxInfo(IP_QPIPE_LIB::TStatus status, const IP_QPIPE_LIB::TPipeRxParams& params)
{
    printf("\n");
    printf("----- rx pipe -----\n");
    printf("key:       %6s\n",params.pipeKey);
    printf("pipeId:    %6d\n",params.pipeId);
    printf("status:    %6d\n",status);
    printf("isCreated: %6d\n",params.isCreated);
    printf("\n");
    printf("chunkNum:  %6d\n",params.pipeInfo.chunkNum);
    printf("chunkSize: %6d\n",params.pipeInfo.chunkSize);
    printf("\n");
    printf("txReady:   %6d\n",params.pipeInfo.txReady);
    for(auto k = 0; k < params.pipeInfo.MaxRxNum; ++k) {
        printf("rxReady[%1d]: %5d\n",k,params.pipeInfo.rxReady[k]);
    }
    printf("-------------------\n");
    printf("\n");
}
