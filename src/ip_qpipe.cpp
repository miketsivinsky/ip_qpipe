//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include <cstdio>
#include <cstring>
#include <type_traits> /*TEST*/

#include <QtGlobal>
#include <QDebug>

//------------------------------------------------------------------------------

#include "ip_qpipe.h"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

bool operator==(const TPipeView::TControlBlock& left, const TPipeView::TControlBlock& right)
{
    if(left.txBufEmpty != right.txBufEmpty)
        return false;
    if(left.txBufIdx != right.txBufIdx)
        return false;
    if(left.txGblIdx != right.txGblIdx)
        return false;

    if((left.chunkNum != right.chunkNum) || (left.chunkSize != right.chunkSize))
        return false;
    if(left.txReady != right.txReady)
        return false;
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        if(left.rxReady[k] !=  right.rxReady[k])
            return false;
    }
    return true;
}

//------------------------------------------------------------------------------
TPipeView::TControlBlock::TControlBlock()
{
    txBufIdx   = 0;
    txGblIdx   = 0;
    txBufEmpty = 1;
    chunkSize  = 0;
    chunkNum   = 0;
    txReady    = 0;
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        rxReady[k] = 0;
    }
}

//------------------------------------------------------------------------------
TPipeView::TControlBlock& TPipeView::TControlBlock::operator=(const TControlBlock& right)
{
    if(this == &right)
        return *this;

    txBufEmpty = right.txBufEmpty;
    txGblIdx   = right.txGblIdx;
    txBufIdx   = right.txBufIdx;

    chunkSize = right.chunkSize;
    chunkNum  = right.chunkNum;
    txReady   = right.txReady;
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        rxReady[k] = right.rxReady[k];
    }
    return *this;
}

//------------------------------------------------------------------------------
void TPipeView::TControlBlock::printInfo(TControlBlock& controlBlock)
{
    printf("\n");
    printf("--- pipe control block info ---\n");
    printf("txBufEmpty: %5d\n",controlBlock.txBufEmpty);
    printf("txBufIdx:  %6d\n",controlBlock.txBufIdx);
    printf("txGblIdx:  %6d\n",controlBlock.txGblIdx);
    printf("chunkNum:  %6d\n",controlBlock.chunkNum);
    printf("chunkSize: %6d\n",controlBlock.chunkSize);
    printf("txReady:   %6d\n",controlBlock.txReady);
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        printf("rxReady[%1d]: %5d\n",k,controlBlock.rxReady[k]);
    }
    printf("-------------------------------\n");
    printf("\n");
}

//------------------------------------------------------------------------------
void TPipeView::TControlBlock::initTxView(TPipeView::TControlBlock& controlBlock, uint32_t chunkSize, uint32_t chunkNum)
{
    controlBlock.txBufEmpty = 1;
    controlBlock.txGblIdx   = 0;
    controlBlock.txBufIdx   = 0;

    controlBlock.chunkSize = chunkSize;
    controlBlock.chunkNum  = chunkNum;
    controlBlock.txReady   = 1;
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        controlBlock.rxReady[k] = 0;
    }
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeView::TControlBlock::attachTxView(TControlBlock& controlBlock, uint32_t chunkSize, uint32_t chunkNum)
{
    if(controlBlock.txReady)
        return IP_QPIPE_LIB::AttachTxExistError;
    if((controlBlock.chunkSize == 0) && (controlBlock.chunkNum == 0)) {
        controlBlock.chunkSize = chunkSize;
        controlBlock.chunkNum  = chunkNum;
    } else {
        if((controlBlock.chunkSize != chunkSize) || (controlBlock.chunkNum != chunkNum)) {
            return IP_QPIPE_LIB::AttachTxParamsError;
        }
    }
    controlBlock.txBufEmpty = 1;
    controlBlock.txGblIdx   = 0;
    controlBlock.txBufIdx   = 0;

    controlBlock.txReady    = 1;
    return IP_QPIPE_LIB::Ok;
}

//------------------------------------------------------------------------------
void TPipeView::TControlBlock::initRxView(TPipeView::TControlBlock& controlBlock)
{
    controlBlock.chunkSize  = 0;
    controlBlock.chunkNum   = 0;
    controlBlock.txReady    = 0;
    controlBlock.rxReady[0] = 1;
    for(auto k = 1; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        controlBlock.rxReady[k] = 0;
    }
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeView::TControlBlock::attachRxView(TControlBlock& controlBlock,int& rxId)
{
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        if(controlBlock.rxReady[k] == 0) {
            controlBlock.rxReady[k] = 1;
            rxId = k;
            return IP_QPIPE_LIB::Ok;
        }
    }
    return IP_QPIPE_LIB::AttachRxExistError;
}

//------------------------------------------------------------------------------
bool TPipeView::TControlBlock::isRxPresent(TControlBlock& controlBlock)
{
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        if(controlBlock.rxReady[k])
            return true;
    }
    return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
QString TPipeViewRxNotifier::genKey(const TPipeView& pipeView, int rxId)
{
    if(rxId == -1)
        return QString::number(pipeView.key()) + QString("_sem_pipe_rx_");
    else
        return QString::number(pipeView.key()) + QString("_sem_pipe_rx_") + QString::number(rxId);
}


//------------------------------------------------------------------------------
TPipeViewRxNotifier::TPipeViewRxNotifier(TPipeViewRx& pipeViewRx) :
                                                                    mExit(false),
                                                                    mGblSem(genKey(pipeViewRx),0,QSystemSemaphore::Create),
                                                                    mPipeViewRx(pipeViewRx)
{
    // TPipeViewRxNotifier constructor
}

//------------------------------------------------------------------------------
void TPipeViewRxNotifier::setKeyPipeId(int rxId)
{
    mGblSem.setKey(genKey(mPipeViewRx,rxId),0,QSystemSemaphore::Create);
}

//------------------------------------------------------------------------------
void TPipeViewRxNotifier::run()
{
    while(!mExit) {
        mGblSem.acquire();
        if(mExit)
            return;

        IP_QPIPE_LIB::TTxEvent txEvent = mPipeViewRx.whatTxEvent();

        //--- IP_QPIPE_LIB::TxConnected
        if(txEvent == IP_QPIPE_LIB::TxConnected) {
           if(mPipeViewRx.mDataBlockData) {
                mPipeViewRx.dataBlockOff();
                qDebug() << "W: [rx pipe] TxConnected received, but DataBlock is ON; key:" << mPipeViewRx.key() << "id:" << mPipeViewRx.id();
           }
           if(mPipeViewRx.dataBlockOn()) {
               mPipeViewRx.syncRxGblIdx();
               mPipeViewRx.mRxSem.acquire(mPipeViewRx.mRxSem.available());
               qDebug() << "I: [rx pipe] TxConnected received; dataBlock ON; key:" << mPipeViewRx.key() << "id:" << mPipeViewRx.id();
           } else {
               qDebug() << "E: [rx pipe] TxConnected received; dataBlock ON; key:" << mPipeViewRx.key() << "id:" << mPipeViewRx.id();
               return;
           }
        }

        //--- IP_QPIPE_LIB::TxTransfer
        if((txEvent == IP_QPIPE_LIB::TxTransfer) && !mPipeViewRx.mDataBlockData) {
            if(mPipeViewRx.dataBlockOn()) {
                mPipeViewRx.syncRxGblIdx(1);
                mPipeViewRx.mRxSem.acquire(mPipeViewRx.mRxSem.available());
                qDebug() << "W: [rx pipe] TxTransfer received; dataBlock ON; key:" << mPipeViewRx.key() << "id:" << mPipeViewRx.id();
            } else {
                qDebug() << "E: [rx pipe] TxTransfer received; dataBlock ON; key:" << mPipeViewRx.key() << "id:" << mPipeViewRx.id();
                return;
            }
        }

        if(txEvent == IP_QPIPE_LIB::TxDisconnected) {
            mPipeViewRx.dataBlockOff();
            /*DEBUG*/ mPipeViewRx.mControlBlockCache = mPipeViewRx.getControlBlockView(); // not quarded
            qDebug() << "I: [rx pipe] TxDisconnected received; dataBlock OFF; key:" << mPipeViewRx.key() << "id:" << mPipeViewRx.id()
                     << "txGblIdx:" << mPipeViewRx.mControlBlockCache.txGblIdx << "rxGblIdx:" << mPipeViewRx.mRxGblIdx;
        }

        if((txEvent == IP_QPIPE_LIB::TxError) && mPipeViewRx.mDataBlockData) {
            mPipeViewRx.dataBlockOff();
            qDebug() << "W: [rx pipe] TxError received; dataBlock OFF; key:" << mPipeViewRx.key() << "id:" << mPipeViewRx.id();
        }

        /*DEBUG*/ //mPipeViewRx.mDataBlock.lock(QString("1502_data"));
        /*DEBUG*/ //mPipeViewRx.mDataBlock.unlock(QString("1502_data"));

        //--- send notify (callback)
        if(mPipeViewRx.mNotifyFunc) {
             mPipeViewRx.mControlBlockCache = mPipeViewRx.getControlBlockView(); // not quarded
            (*mPipeViewRx.mNotifyFunc)(mPipeViewRx.key(),txEvent,mPipeViewRx.id(),mPipeViewRx.mControlBlockCache);
        }

        //--- rx semaphore singaling
        if((txEvent == IP_QPIPE_LIB::TxTransfer) && mPipeViewRx.isRxSemSignalEna()) {
            mPipeViewRx.mRxSem.release();
        }
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TPipeView::TPipeView(unsigned pipeKey) :
                                          mControlBlock(),
                                          mControlBlockData(0),
                                          mDataBlock(),
                                          mDataBlockData(0),
                                          mStatus(IP_QPIPE_LIB::NotInit),
                                          mLastError(IP_QPIPE_LIB::NotInit),
                                          mControlBlockCache(),
                                          mKey(pipeKey)
{
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [TPipeView constructor]   key:" << key();
    #endif
}

//------------------------------------------------------------------------------
TPipeView::~TPipeView()
{
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [TPipeView destructor] key:" << key();
    #endif
}

//------------------------------------------------------------------------------
bool TPipeView::attachControlBlock()
{
    if(!mControlBlock.attach(QSharedMemory::ReadWrite)) {
        mLastError = mStatus = IP_QPIPE_LIB::AttachError;
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
bool TPipeView::getControlBlockDataPtr()
{
    if(!(mControlBlockData = mControlBlock.data())) {
        mControlBlock.detach();
        mLastError = mStatus = IP_QPIPE_LIB::DataAccessError;
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
bool TPipeView::attachDataBlock(QSharedMemory::AccessMode accessMode)
{
    if(!mDataBlock.attach(accessMode)) {
        mLastError = mStatus = IP_QPIPE_LIB::AttachError;
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
bool TPipeView::getDataBlockDataPtr()
{
    if(!(mDataBlockData = mDataBlock.data())) {
        mDataBlock.detach();
        mLastError = mStatus = IP_QPIPE_LIB::DataAccessError;
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
TPipeView::TChunk TPipeView::getChunk(uint32_t idx)
{
    TChunk chunk = { 0, 0 };
    if(mStatus == IP_QPIPE_LIB::Ok) {
        void* chunkPtr = static_cast<uint8_t*>(mDataBlockData) + idx*(sizeof(TChunkHeader) + mControlBlockCache.chunkSize);
        chunk.chunkHeader = static_cast<TChunkHeader*>(chunkPtr);
        chunk.chunkData   = static_cast<uint8_t*>(chunkPtr) + sizeof(TChunkHeader);
    }
    return chunk;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TPipeViewTx::TPipeViewTx(IP_QPIPE_LIB::TPipeTxParams& params) : TPipeView(params.pipeKey)
{
    mControlBlock.setKey(QString::number(params.pipeKey) +QString("_control"));
    mDataBlock.setKey(QString::number(params.pipeKey) +QString("_data"));

    //---
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        mSem[k] = 0;
    }

    //---
    if(!activatePipe(params))
        return;

    //---
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        mSem[k] = new QSystemSemaphore(TPipeViewRxNotifier::genKey(*this,k),0,QSystemSemaphore::Create);
    }

    //---
    mControlBlockCache = getControlBlockView(); // not quarded
    params.pipeInfo    = mControlBlockCache;
    notifyRx(mControlBlockCache);

    /*TEST*/ // qDebug() << "is_standard_layout<TPipeView::TControlBlock>" << std::is_standard_layout<TPipeView::TControlBlock>::value;
}

//------------------------------------------------------------------------------
TPipeViewTx::~TPipeViewTx()
{
    if(mControlBlockData) {
        { // locked
            TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
            TControlBlock& controlBlockView = getControlBlockView();
            controlBlockView.txReady = 0;
            notifyRx(controlBlockView);
        } // unlocked
        for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
            delete mSem[k];
        }
    }
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [TPipeViewTx destructor] key:" << key();
    #endif
}

//------------------------------------------------------------------------------
bool TPipeViewTx::activatePipe(IP_QPIPE_LIB::TPipeTxParams& params)
{
    uint32_t chunkSize = params.pipeInfo.chunkSize;
    uint32_t chunkNum  = params.pipeInfo.chunkNum;

    //--- pipe exist, viewTx attached
    if(!mControlBlock.create(sizeof(TPipeView::TControlBlock),QSharedMemory::ReadWrite)) {
        if(mControlBlock.error() != QSharedMemory::AlreadyExists) {
            #if defined(IP_QPIPE_PRINT_DEBUG_ERROR)
                qDebug() << "[ERROR] [TPipeViewTx activatePipe] at QSharedMemory::create";
            #endif
            mLastError = mStatus = IP_QPIPE_LIB::CreateError;
            return false;
        }
        if(!attachControlBlock())
            return false;
        if(!getControlBlockDataPtr())
            return false;
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        if(!activateDataBlock(params))
            return false;
        if((mLastError = mStatus = TControlBlock::attachTxView(getControlBlockView(),chunkSize, chunkNum)) != IP_QPIPE_LIB::Ok)
            return false;
        params.isCreated = false;
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "I: [tx pipe] attached; key:" << key();
            /*DEBUG*/ TControlBlock::printInfo(getControlBlockView());
        #endif
    } else { //--- pipe not exist, viewTx created
        if(!getControlBlockDataPtr())
            return false;
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        if(!activateDataBlock(params))
            return false;
        TControlBlock::initTxView(getControlBlockView(),chunkSize, chunkNum);
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "I: [tx pipe] created; key:" << key();
            /*DEBUG*/ TControlBlock::printInfo(getControlBlockView());
        #endif
        params.isCreated = true;
        mLastError = mStatus = IP_QPIPE_LIB::Ok;
    }
    return true;
}

//------------------------------------------------------------------------------
bool TPipeViewTx::activateDataBlock(IP_QPIPE_LIB::TPipeTxParams& params)
{
    uint32_t chunkSize = params.pipeInfo.chunkSize;
    uint32_t chunkNum  = params.pipeInfo.chunkNum;

    //--- data pipe exist, viewTx attached
    if(!mDataBlock.create(chunkNum*(sizeof(TPipeView::TChunkHeader) + chunkSize),QSharedMemory::ReadWrite)) {
        if(mDataBlock.error() != QSharedMemory::AlreadyExists) {
            #if defined(IP_QPIPE_PRINT_DEBUG_ERROR)
                qDebug() << "[ERROR] [TPipeViewTx activateDataBlock] at QSharedMemory::create";
            #endif
            mLastError = mStatus = IP_QPIPE_LIB::CreateError;
            return false;
        }
        if(!attachDataBlock())
            return false;
        if(!getDataBlockDataPtr())
            return false;
        qDebug() << "W: [tx pipe] DataBlock exist - attached; key:" << key();
    } else { //--- data pipe not exist, viewTx created
        if(!getDataBlockDataPtr())
            return false;
        qDebug() << "I: [tx pipe] DataBlock not exist - created; key:" << key();
    }
    return true;
}

//------------------------------------------------------------------------------
unsigned TPipeViewTx::notifyRx(const TPipeView::TControlBlock& controlBlock)
{
    unsigned num = 0;
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        if(controlBlock.rxReady[k]) {
            ++num;
            mSem[k]->release();
        }
    }
    return num;
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewTx::sendData(IP_QPIPE_LIB::TPipeTxTransfer& txTransfer)
{
    // 0. check pipe ok
    if(!isPipeOk()) {
        return mStatus;
    }

    mLastError = IP_QPIPE_LIB::Ok;

    // 1. check present of rxPipe views (if need)
    if(txTransfer.rxMustBePresent && !isRxPresent()) {
        mLastError = IP_QPIPE_LIB::RxNotPresentError;
        return mLastError;
    }

    // 2. check len & bufPtr
    if((txTransfer.dataLen > mControlBlockCache.chunkSize ) || (txTransfer.dataLen == 0) || (!txTransfer.dataBuf)) {
        mLastError = IP_QPIPE_LIB::DataParamError;
        return mLastError;
    }

    // 3. lock control & data
    TLock lockControlBlock(mControlBlock);
    TLock lockDataBlock(mDataBlock);

    // 4. get chunk & write data
    TControlBlock& controlBlockView = getControlBlockView();
    TChunk chunk = getChunk(controlBlockView.txBufIdx);
    if(!chunk.chunkData || !chunk.chunkHeader) {
        mLastError = IP_QPIPE_LIB::DataAccessError;
        return mLastError;
    }
    std::memcpy(chunk.chunkData,txTransfer.dataBuf,txTransfer.dataLen);
    chunk.chunkHeader->chunkLen = txTransfer.dataLen;

    // 5. modify txBufIdx, txGblIdx, txBufEmpty
    if(controlBlockView.txBufEmpty)
        controlBlockView.txBufEmpty = 0;
    controlBlockView.txBufIdx = ((controlBlockView.txBufIdx + 1) == controlBlockView.chunkNum) ? 0 : (controlBlockView.txBufIdx + 1);
    ++controlBlockView.txGblIdx;
    mControlBlockCache = controlBlockView;

    // 6. send notification to receivers
    notifyRx(mControlBlockCache);

    // 7. only for debug purposes, not need for real work
    txTransfer.txBufIdx = controlBlockView.txBufIdx;
    txTransfer.txGblIdx = controlBlockView.txGblIdx;

    return mLastError;
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewTx::sendData(IP_QPIPE_LIB::TPipeTxTransferFuncObj& txTransfer)
{
    // 0. check pipe ok
    if(!isPipeOk()) {
        return mStatus;
    }

    mLastError = IP_QPIPE_LIB::Ok;

    // 1. check present of rxPipe views (if need)
    if(txTransfer.rxMustBePresent && !isRxPresent()) {
        mLastError = IP_QPIPE_LIB::RxNotPresentError;
        return mLastError;
    }

    // 2. check obj & transferFunc
    if(!txTransfer.obj || !txTransfer.transferFunc) {
        mLastError = IP_QPIPE_LIB::DataParamError;
        return mLastError;
    }

    // 3. lock control & data
    TLock lockControlBlock(mControlBlock);
    TLock lockDataBlock(mDataBlock);

    // 4. get chunk & write data
    TControlBlock& controlBlockView = getControlBlockView();
    TChunk chunk = getChunk(controlBlockView.txBufIdx);
    if(!chunk.chunkData || !chunk.chunkHeader) {
        mLastError = IP_QPIPE_LIB::DataAccessError;
        return mLastError;
    }
    uint32_t dataLen = (*txTransfer.transferFunc)(txTransfer.obj,chunk.chunkData,controlBlockView.chunkSize);
    txTransfer.dataLen  = dataLen; // only for debug purposes, not need for real work
    if((dataLen == 0) || (dataLen > controlBlockView.chunkSize)) {
        mLastError = IP_QPIPE_LIB::DataParamError;
        return mLastError;
    }
    chunk.chunkHeader->chunkLen = dataLen;

    // 5. modify txBufIdx, txGblIdx, txBufEmpty
    if(controlBlockView.txBufEmpty)
        controlBlockView.txBufEmpty = 0;
    controlBlockView.txBufIdx = ((controlBlockView.txBufIdx + 1) == controlBlockView.chunkNum) ? 0 : (controlBlockView.txBufIdx + 1);
    ++controlBlockView.txGblIdx;
    mControlBlockCache = controlBlockView;

    // 6. send notification to receivers
    notifyRx(mControlBlockCache);

    // 7. only for debug purposes, not need for real work
    txTransfer.txBufIdx = controlBlockView.txBufIdx;
    txTransfer.txGblIdx = controlBlockView.txGblIdx;
    txTransfer.dataLen  = dataLen;

    return mLastError;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TPipeViewRx::TPipeViewRx(IP_QPIPE_LIB::TPipeRxParams& params) : TPipeView(params.pipeKey),
                                                                mId(-1),
                                                                mNotifier(*this),
                                                                mNotifyFunc(params.pipeRxNotifyFunc),
                                                                mRxGblIdx(0),
                                                                mRxSem(0)
{
    mControlBlock.setKey(QString::number(params.pipeKey) + QString("_control"));
    //mDataBlock.setKey(QString::number(params.pipeKey) + QString("_data"));

    //---
    if(!activatePipe(params))
        return;

    //---
    mNotifier.setKeyPipeId(id());
    syncRxGblIdx();

    params.pipeId   = id();
    params.pipeInfo = mControlBlockCache;
    mNotifier.start();
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [TPipeViewRx constructor]   key:" << key();
    #endif
}

//------------------------------------------------------------------------------
TPipeViewRx::~TPipeViewRx()
{
    if(!mNotifier.stop()) {
        qDebug() << "[ERROR] [TPipeViewRx destructor] TPipeViewRxNotifier finish timeot expired";
    }

    if(mControlBlockData && (id() != -1)) {
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        TControlBlock& controlBlockView = getControlBlockView();
        controlBlockView.rxReady[mId] = 0;
    }
    mStatus = IP_QPIPE_LIB::PipeNotExistError;
    mRxSem.release();

    TQtMutexGuard::TLocker lock(mInstanceGuard);
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "I: [TPipeViewRx destructor] key:" << key() << "id:" << id(); // << "thread id:" << QThread::currentThreadId();
    #endif
}

//------------------------------------------------------------------------------
bool TPipeViewRx::activatePipe(IP_QPIPE_LIB::TPipeRxParams& params)
{
    //--- pipe exist, viewRx attached
    if(!mControlBlock.create(sizeof(TPipeView::TControlBlock),QSharedMemory::ReadWrite)) {
        if(mControlBlock.error() != QSharedMemory::AlreadyExists) {
            #if defined(IP_QPIPE_PRINT_DEBUG_ERROR)
                qDebug() << "[ERROR] [TPipeViewTx activatePipe] at QSharedMemory::create";
            #endif
            mLastError = mStatus = IP_QPIPE_LIB::CreateError;
            return false;
        }
        if(!attachControlBlock())
            return false;
        if(!getControlBlockDataPtr())
            return false;
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        /*DEBUG*/ // TControlBlock::printInfo(getControlBlockView());
        if((mLastError = mStatus = TControlBlock::attachRxView(getControlBlockView(),mId)) != IP_QPIPE_LIB::Ok)
            return false;
        params.isCreated = false;
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "I: [rx pipe] attached; key:" << key() << "id:" << id();
        #endif
    } else { //--- pipe not exist, viewRx created
        if(!getControlBlockDataPtr())
            return false;
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        TControlBlock::initRxView(getControlBlockView());
        mId = 0;
        params.isCreated = true;
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "I: [rx pipe] created; key:" << key() << "id:" << id();
        #endif
        mLastError = mStatus = IP_QPIPE_LIB::Ok;
    }
    return true;
}

//------------------------------------------------------------------------------
bool TPipeViewRx::dataBlockOnNoLock()
{
    mDataBlockData = 0;
    mDataBlock.setKey(QString::number(key()) + QString("_data"));
    if(!attachDataBlock(QSharedMemory::ReadOnly))
        return false;
    if(!getDataBlockDataPtr())
        return false;
    return true;
}

//------------------------------------------------------------------------------
bool TPipeViewRx::dataBlockOffNoLock(bool semReset)
{
    if(semReset) {
        mRxSem.acquire(mRxSem.available());
    }
    mDataBlockData = 0;
    mDataBlock.setKey(QString());
    return true;
}

//------------------------------------------------------------------------------
bool TPipeViewRx::dataBlockOn()
{
    TQtMutexGuard::TLocker lock(mDataBlockGuard);
    return dataBlockOnNoLock();
}

//#define DEBUG_DATA_BLOCK_OFF
//------------------------------------------------------------------------------
bool TPipeViewRx::dataBlockOff()
{
    #if !defined(DEBUG_DATA_BLOCK_OFF)
        mRxSem.acquire(mRxSem.available());
    #endif
    TQtMutexGuard::TLocker lock(mDataBlockGuard);
    return dataBlockOffNoLock(false);
}


//------------------------------------------------------------------------------
bool TPipeViewRx::isRxSemSignalEna()
{
    return (!mControlBlockCache.txBufEmpty && (mRxSem.available() < static_cast<int>(mControlBlockCache.chunkNum)));
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TTxEvent TPipeViewRx::whatTxEvent()
{
    IP_QPIPE_LIB::TTxEvent txEvent = IP_QPIPE_LIB::TxError;

    TLock lockControlBlock(mControlBlock);
    TControlBlock& controlBlockView = getControlBlockView();

    if((mControlBlockCache.txReady == 0) && (controlBlockView.txReady != 0))
        txEvent = IP_QPIPE_LIB::TxConnected;
    if((mControlBlockCache.txReady != 0) && (controlBlockView.txReady == 0))
        txEvent = IP_QPIPE_LIB::TxDisconnected;
    if((mControlBlockCache.txReady != 0) && (controlBlockView.txReady != 0))
        txEvent = IP_QPIPE_LIB::TxTransfer;

    mControlBlockCache = controlBlockView;
    return txEvent;
}

//------------------------------------------------------------------------------
void TPipeViewRx::syncRxGblIdx(uint32_t offset)
{
    mControlBlockCache = getControlBlockView(); // not quarded
    mRxGblIdx = mControlBlockCache.txGblIdx - offset;
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewRx::readData(IP_QPIPE_LIB::TPipeRxTransfer& rxTransfer, int timeout)
{
    TQtMutexGuard::TLocker lock(mInstanceGuard);

    // 0. check pipe ok
    if(!isPipeOk()) {
        return mStatus;
    }

    mLastError = IP_QPIPE_LIB::Ok;

    // 1. wait for signal
    if(!mRxSem.tryAcquire(1,timeout)) {
        mLastError = mDataBlockData ? IP_QPIPE_LIB::TimeoutError : IP_QPIPE_LIB::TxPipeNotPresent;
        return mLastError;
    } else {
         if(!isPipeOk()) {
             return mStatus;
         }
    }

    // 2. lock control & data
    TLock lockControlBlock(mControlBlock);
    TLock lockDataBlock(mDataBlock);
    mControlBlockCache = getControlBlockView();

    // 3. compute idxDelta & idxNormDelta ('normalized' to buf size)
    uint32_t idxDelta = mControlBlockCache.txGblIdx - mRxGblIdx;
    if(idxDelta == 0) {
        mLastError = IP_QPIPE_LIB::NoRxDataError;
        return mLastError;
    }
    uint32_t idxNormDelta = (idxDelta >= mControlBlockCache.chunkNum) ? (mControlBlockCache.chunkNum - 1) : idxDelta;

    // 4. correct RxSem signal number
    int32_t signalSemDelta = mRxSem.available() - idxNormDelta;

    if(signalSemDelta > 0) {
        mRxSem.acquire(signalSemDelta);
    }

    // 5. advance "local" (buf) rx idx
    uint32_t rxBufIdx = computeRxBufIdx(idxNormDelta);

    // 6. advance "global" rx idx
    if(idxDelta >= mControlBlockCache.chunkNum)
        mRxGblIdx = mControlBlockCache.txGblIdx - mControlBlockCache.chunkNum + 1;
    ++mRxGblIdx;

    // 7. read data from chunk
    TChunk chunk = getChunk(rxBufIdx);
    if(!chunk.chunkData || !chunk.chunkHeader) {
        mLastError = IP_QPIPE_LIB::DataAccessError;
        return mLastError;
    }

    // 8. check chunk data len
    rxTransfer.dataLen = chunk.chunkHeader->chunkLen;
    if((chunk.chunkHeader->chunkLen > rxTransfer.dataLen) || (chunk.chunkHeader->chunkLen == 0)) {
        mLastError = IP_QPIPE_LIB::RxDataLenError;
        return mLastError;
    }

    // 9. copy data
    //--- it's feature (rxTransfer.dataBuf == 0) for speculative reading
    if(rxTransfer.dataBuf) {
        std::memcpy(rxTransfer.dataBuf,chunk.chunkData,chunk.chunkHeader->chunkLen);
    }

    return mLastError;
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewRx::readData(IP_QPIPE_LIB::TPipeRxTransferFuncObj& rxTransfer, int timeout)
{
    TQtMutexGuard::TLocker lock(mInstanceGuard);

    // 0. check pipe ok
    if(!isPipeOk()) {
        return mStatus;
    }

    mLastError = IP_QPIPE_LIB::Ok;

    if((mRxSem.available() < 4) && mDataBlockData) {
        return mLastError;
    }

    // 1. wait for signal
    if(!mRxSem.tryAcquire(1,timeout)) {
        mLastError = mDataBlockData ? IP_QPIPE_LIB::TimeoutError : IP_QPIPE_LIB::TxPipeNotPresent;
        return mLastError;
    } else {
        if(!isPipeOk()) {
            return mStatus;
        }
    }

    // 2. lock control & data
    TLock lockControlBlock(mControlBlock);
    TLock lockDataBlock(mDataBlock);
    mControlBlockCache = getControlBlockView();

    // 3. compute idxDelta & idxNormDelta ('normalized' to buf size)
    uint32_t idxDelta = mControlBlockCache.txGblIdx - mRxGblIdx;
    if(idxDelta == 0) {
        mLastError = IP_QPIPE_LIB::NoRxDataError;
        qDebug() << "[DEBUG] [NoRxDataError] pipeKey:" << key() << "txGblIdx:" << mControlBlockCache.txGblIdx << "rxGblIdx:" << mRxGblIdx;
        return mLastError;
    }
    uint32_t idxNormDelta = (idxDelta >= mControlBlockCache.chunkNum) ? (mControlBlockCache.chunkNum - 1) : idxDelta;

    // 4. correct RxSem signal number
    int32_t signalSemDelta = mRxSem.available() - idxNormDelta;
    if(signalSemDelta > 0) {
        mRxSem.acquire(signalSemDelta);
    }

    // 5. advance "local" (buf) rx idx
    uint32_t rxBufIdx = computeRxBufIdx(idxNormDelta);

    // 6. advance "global" rx idx
    if(idxDelta >= mControlBlockCache.chunkNum)
        mRxGblIdx = mControlBlockCache.txGblIdx - mControlBlockCache.chunkNum + 1;
    ++mRxGblIdx;

    // 7. read data from chunk
    TChunk chunk = getChunk(rxBufIdx);
    if(!chunk.chunkData || !chunk.chunkHeader) {
        mLastError = IP_QPIPE_LIB::DataAccessError;
        return mLastError;
    }

    // 8. check chunk data len
    rxTransfer.dataLen = chunk.chunkHeader->chunkLen;
    if(chunk.chunkHeader->chunkLen == 0) {
        mLastError = IP_QPIPE_LIB::RxDataLenError;
        return mLastError;
    }

    // 9. check chunk data len
    if(!rxTransfer.transferFunc || !rxTransfer.obj ||  !(*rxTransfer.transferFunc)(rxTransfer.obj,chunk.chunkData,chunk.chunkHeader->chunkLen)) {
        mLastError = IP_QPIPE_LIB::RxDataFuncObjError;
        return mLastError;
    }

    return mLastError;
}

//------------------------------------------------------------------------------
uint32_t TPipeViewRx::computeRxBufIdx(uint32_t idxNormDelta) const
{
    uint32_t rxBufIdx;
    if(idxNormDelta <= mControlBlockCache.txBufIdx)
        rxBufIdx = mControlBlockCache.txBufIdx - idxNormDelta;
    else
        rxBufIdx = mControlBlockCache.chunkNum - (idxNormDelta - mControlBlockCache.txBufIdx);
    return rxBufIdx;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TPipeViewPool::~TPipeViewPool()
{
    for(auto pipeViewTx = txPool().begin(); pipeViewTx != txPool().end(); ++pipeViewTx) {
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "[INFO] [TPipeViewPool destructor] tx pipe with key:" << pipeViewTx->first << "will be deleted";
        #endif
        delete pipeViewTx->second;
    }
    for(auto pipeViewRx = rxPool().begin(); pipeViewRx != rxPool().end(); ++pipeViewRx) {
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "[INFO] [TPipeViewPool destructor] rx pipe with key:" << pipeViewRx->first << "will be deleted";
        #endif
        delete pipeViewRx->second;
    }
}

//------------------------------------------------------------------------------
TPipeView* TPipeViewPool::getPipeView(unsigned key, TPipeViewPoolMap& pool)
{
    auto pipeView = pool.find(key);
    return (pipeView == pool.end()) ? 0 : pipeView->second;
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewPool::createPipeViewTx(IP_QPIPE_LIB::TPipeTxParams& params)
{
    if(isPipeViewTxExist(params.pipeKey)) {
        return IP_QPIPE_LIB::PipeExistError;
    }

    TPipeViewTx* pipeView = new TPipeViewTx(params);
    if(!pipeView->isPipeOk()) {
        IP_QPIPE_LIB::TStatus err = pipeView->error();
        delete pipeView;
        return err;
    }
    txPool().insert(std::pair<unsigned,TPipeView*>(pipeView->key(),pipeView));
    return IP_QPIPE_LIB::Ok;
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewPool::createPipeViewRx(IP_QPIPE_LIB::TPipeRxParams& params)
{
    if(isPipeViewRxExist(params.pipeKey)) {
        return IP_QPIPE_LIB::PipeExistError;
    }

    TPipeViewRx* pipeView = new TPipeViewRx(params);
    if(!pipeView->isPipeOk()) {
        IP_QPIPE_LIB::TStatus err = pipeView->error();
        delete pipeView;
        return err;
    }
    rxPool().insert(std::pair<unsigned,TPipeView*>(pipeView->key(),pipeView));
    return IP_QPIPE_LIB::Ok;
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewPool::deletePipeView(unsigned pipeKey, TPipeViewPoolMap& pool)
{
    TPipeView* pipeView = getPipeView(pipeKey, pool);
    if(pipeView) {
        pool.erase(pipeKey);
        delete pipeView;
        return IP_QPIPE_LIB::Ok;
    } else {
        return IP_QPIPE_LIB::PipeNotExistError;
    }
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewPool::sendData(IP_QPIPE_LIB::TPipeTxTransfer& txTransfer)
{
    TPipeViewTx* pipeTxView = static_cast<TPipeViewTx*>(getPipeView(txTransfer.pipeKey,txPool()));
    if(!pipeTxView) {
        return IP_QPIPE_LIB::PipeNotExistError;
    }
    return pipeTxView->sendData(txTransfer);
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewPool::sendData(IP_QPIPE_LIB::TPipeTxTransferFuncObj& txTransfer)
{
    TPipeViewTx* pipeTxView = static_cast<TPipeViewTx*>(getPipeView(txTransfer.pipeKey,txPool()));
    if(!pipeTxView) {
        return IP_QPIPE_LIB::PipeNotExistError;
    }
    return pipeTxView->sendData(txTransfer);
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewPool::readData(IP_QPIPE_LIB::TPipeRxTransfer& rxTransfer, int timeout)
{
    TPipeViewRx* pipeRxView = static_cast<TPipeViewRx*>(getPipeView(rxTransfer.pipeKey,rxPool()));
    if(!pipeRxView) {
        return IP_QPIPE_LIB::PipeNotExistError;
    }
    return pipeRxView->readData(rxTransfer,timeout);
}

IP_QPIPE_LIB::TStatus TPipeViewPool::readData(IP_QPIPE_LIB::TPipeRxTransferFuncObj& rxTransfer, int timeout)
{
    TPipeViewRx* pipeRxView = static_cast<TPipeViewRx*>(getPipeView(rxTransfer.pipeKey,rxPool()));
    if(!pipeRxView) {
        return IP_QPIPE_LIB::PipeNotExistError;
    }
    return pipeRxView->readData(rxTransfer,timeout);
}
