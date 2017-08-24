#include <cstdlib>

#include <QThread>

#include "../common/RawStreamTester.h"
#include "ip_qpipe_lib.h"

//------------------------------------------------------------------------------
void printPipeTxInfo(IP_QPIPE_LIB::TStatus status, const IP_QPIPE_LIB::TPipeTxParams& params);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    unsigned long sTime = 0;
    if(argc > 1) {
        sTime = atol(argv[1]);
    }

    //---
    IP_QPIPE_LIB::TPipeTxParams txParams;
    txParams.pipeKey            = 2307;
    txParams.pipeInfo.chunkNum  = 32;
    txParams.pipeInfo.chunkSize = 1024;

    IP_QPIPE_LIB::TStatus status = IP_QPIPE_LIB::createPipeViewTx(txParams);
    printPipeTxInfo(status,txParams);

    QThread::sleep(sTime);
    return 0;
}

//------------------------------------------------------------------------------
void printPipeTxInfo(IP_QPIPE_LIB::TStatus status, const IP_QPIPE_LIB::TPipeTxParams& params)
{
    printf("\n");
    printf("----- tx pipe -----\n");
    printf("key:       %6d\n",params.pipeKey);
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


