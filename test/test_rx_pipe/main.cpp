#include <cstdlib>

#include <QThread>
#include <QSemaphore>

#include "../common/RawStreamTester.h"
#include "ip_qpipe_lib.h"

//------------------------------------------------------------------------------
const bool     SyncMode    = true;
const unsigned TransferNum = 16;
const unsigned Timeout     = 5000;
const unsigned RxBufSize   = 4*1024;


//------------------------------------------------------------------------------
void printPipeRxInfo(IP_QPIPE_LIB::TStatus status, const IP_QPIPE_LIB::TPipeRxParams& params);
void PipeRxNotifyFunc(unsigned pipeKey, IP_QPIPE_LIB::TTxEvent txEvent, int pipeId, const IP_QPIPE_LIB::TPipeInfo& pipeInfo);

//------------------------------------------------------------------------------
uint8_t    RxBuf[RxBufSize];
QSemaphore ReadFrameSem;

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
    rxParams.pipeKey            = 2307;
    rxParams.pipeRxNotifyFunc   = PipeRxNotifyFunc;

    IP_QPIPE_LIB::TStatus status = IP_QPIPE_LIB::createPipeViewRx(rxParams);
    if(status != IP_QPIPE_LIB::Ok) {
        printf("[ERROR] IP_QPIPE_LIB::createPipeViewRx, status: %2d\n",status);
        return 0;
    }
    printPipeRxInfo(status,rxParams);

    //---
    IP_QPIPE_LIB::TPipeRxTransfer rxTransfer;
    for(auto k = 0u; k < TransferNum; ++k) {
        if(SyncMode || ReadFrameSem.tryAcquire(1,Timeout)) {
            rxTransfer.pipeKey = rxParams.pipeKey;
            rxTransfer.dataBuf = RxBuf;
            rxTransfer.dataLen = RxBufSize;
            status = IP_QPIPE_LIB::readData(rxTransfer);
            if(status != IP_QPIPE_LIB::Ok) {
                printf("[ERROR] IP_QPIPE_LIB::readData, status: %2d\n",status);
                break;
            }
            printf("[INFO] [data read] packet: %6d, size: %6d\n",k,rxTransfer.dataLen);
        } else {
            printf("[WARN] Timeout Expired\n");
            break;
        }
    }
    QThread::sleep(sTime);
    return 0;
}

//------------------------------------------------------------------------------
void printPipeRxInfo(IP_QPIPE_LIB::TStatus status, const IP_QPIPE_LIB::TPipeRxParams& params)
{
    printf("\n");
    printf("----- rx pipe -----\n");
    printf("key:       %6d\n",params.pipeKey);
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

//------------------------------------------------------------------------------
void PipeRxNotifyFunc(unsigned pipeKey, IP_QPIPE_LIB::TTxEvent txEvent, int pipeId, const IP_QPIPE_LIB::TPipeInfo& pipeInfo)
{
    #if 0
        printf("--- PipeRxNotifyFunc ---\n");
        printf("key:       %6d\n",pipeKey);
        printf("txEvent:   %6d\n",txEvent);
        printf("pipeId:    %6d\n",pipeId);
        printf("chunkNum:  %6d\n",pipeInfo.chunkNum);
        printf("chunkSize: %6d\n",pipeInfo.chunkSize);
        printf("\n");
    #endif
}
