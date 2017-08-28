#include <cstdint>
#include <cstdlib>

#include <QThread>

#include "../common/RawStreamTester.h"
#include "ip_qpipe_lib.h"

//------------------------------------------------------------------------------
const unsigned TransferNum = 10;

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
    if(status != IP_QPIPE_LIB::Ok) {
        printf("[ERROR] IP_QPIPE_LIB::createPipeViewTx, status: %2d\n",status);
        return 0;
    }

    //---
    IP_QPIPE_LIB::TPipeTxTransfer txTransfer;
    uint8_t* txBuf = new uint8_t [txParams.pipeInfo.chunkSize];

    for(auto k = 0; k < TransferNum; ++k) {
        //--- parameters setup
        txTransfer.dataBuf         = txBuf;
        txTransfer.dataLen         = txParams.pipeInfo.chunkSize;
        txTransfer.pipeKey         = txParams.pipeKey;
        txTransfer.rxMustBePresent = true;

        status = IP_QPIPE_LIB::sendData(txTransfer);
        if(status != IP_QPIPE_LIB::Ok) {
            printf("[ERROR] IP_QPIPE_LIB::sendData, status: %2d\n",status);
            break;
        }

        printf("-------------------------\n");
        printf("[INFO] data sent to pipe; key: %6d, packet: %6d\n",txTransfer.pipeKey,k);
        printf("[INFO] txBufIdx: %6d\n",txTransfer.txBufIdx);
        printf("[INFO] txGblIdx: %6d\n",txTransfer.txGblIdx);
    }

    //---
    QThread::sleep(sTime);
    delete [] txBuf;
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


