//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include <cstdio>
#include <type_traits> /*TEST*/

#include <QtGlobal>
#include <QDebug>

//------------------------------------------------------------------------------

#include "ip_qpipe.h"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

bool operator==(const TPipeView::TControlBlock& left, const TPipeView::TControlBlock& right)
{
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
    chunkSize = 0;
    chunkNum  = 0;
    txReady   = 0;
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        rxReady[k] = 0;
    }
}

//------------------------------------------------------------------------------
TPipeView::TControlBlock& TPipeView::TControlBlock::operator=(const TControlBlock& right)
{
    if(this == &right)
        return *this;

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
    controlBlock.txReady   = 1;
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
                                                                    mSem(genKey(pipeViewRx),0,QSystemSemaphore::Create),
                                                                    mPipeViewRx(pipeViewRx)
{
    // TPipeViewRxNotifier constructor
}

//------------------------------------------------------------------------------
void TPipeViewRxNotifier::setKeyPipeId(int rxId)
{
    mSem.setKey(genKey(mPipeViewRx.key(),rxId));
}

//------------------------------------------------------------------------------
void TPipeViewRxNotifier::run()
{
    while(!mExit) {
        mSem.acquire();
        if(mExit)
            return;
        IP_QPIPE_LIB::TTxEvent txEvent = mPipeViewRx.whatTxEvent();
        if(mPipeViewRx.mNotifyFunc) {
            (*mPipeViewRx.mNotifyFunc)(mPipeViewRx.key(),txEvent,mPipeViewRx.id(),mPipeViewRx.mControlBlockCache);
        }
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TPipeView::TPipeView(unsigned key) :
                                          mControlBlock(QString::number(key) +QString("_control")),
                                          mControlBlockData(0),
                                          mStatus(IP_QPIPE_LIB::NotInit),
                                          mLastError(IP_QPIPE_LIB::NotInit),
                                          mControlBlockCache(),
                                          mKey(key)
{
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        //qDebug() << "[INFO] [TPipeView constructor]   key:" << key();
    #endif
}

//------------------------------------------------------------------------------
TPipeView::~TPipeView()
{
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        //qDebug() << "[INFO] [TPipeView destructor] key:" << key();
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
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TPipeViewTx::TPipeViewTx(IP_QPIPE_LIB::TPipeTxParams& params) : TPipeView(params.pipeKey)
{
    //---
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        mSem[k] = 0;
    }

    //---
    if(!activateControlBlock(params))
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
bool TPipeViewTx::activateControlBlock(IP_QPIPE_LIB::TPipeTxParams& params)
{
    uint32_t chunkSize = params.pipeInfo.chunkSize;
    uint32_t chunkNum  = params.pipeInfo.chunkNum;

    //--- pipe exist, viewTx attached
    if(!mControlBlock.create(sizeof(TPipeView::TControlBlock),QSharedMemory::ReadWrite)) {
        if(mControlBlock.error() != QSharedMemory::AlreadyExists) {
            #if defined(IP_QPIPE_PRINT_DEBUG_ERROR)
                qDebug() << "[ERROR] [TPipeViewTx activateControlBlock] at QSharedMemory::create";
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
        if((mLastError = mStatus = TControlBlock::attachTxView(getControlBlockView(),chunkSize, chunkNum)) != IP_QPIPE_LIB::Ok)
            return false;
        params.isCreated = false;
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            // qDebug() << "[INFO] [tx][attached] key:" << key();
        #endif
    } else { //--- pipe not exist, viewTx created
        if(!getControlBlockDataPtr())
            return false;
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        TControlBlock::initTxView(getControlBlockView(),chunkSize, chunkNum);
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            // qDebug() << "[INFO] [tx][created] key:" << key();
        #endif
        params.isCreated = true;
        mLastError = mStatus = IP_QPIPE_LIB::Ok;
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
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TPipeViewRx::TPipeViewRx(IP_QPIPE_LIB::TPipeRxParams& params) : TPipeView(params.pipeKey),
                                                                mId(-1),
                                                                mNotifier(*this),
                                                                mNotifyFunc(params.pipeRxNotifyFunc)
{
    //--- pipe exist, viewRx attached
    if(!mControlBlock.create(sizeof(TPipeView::TControlBlock),QSharedMemory::ReadWrite)) {
        if(mControlBlock.error() != QSharedMemory::AlreadyExists) {
            #if defined(IP_QPIPE_PRINT_DEBUG_ERROR)
                qDebug() << "[ERROR] [TPipeViewRx constructor] at QSharedMemory::create";
            #endif
            mLastError = mStatus = IP_QPIPE_LIB::CreateError;
            return;
        }
        if(!attachControlBlock())
            return;
        if(!getControlBlockDataPtr())
            return;
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        /*DEBUG*/ // TControlBlock::printInfo(getControlBlockView());
        if((mLastError = mStatus = TControlBlock::attachRxView(getControlBlockView(),mId)) != IP_QPIPE_LIB::Ok)
            return;
        params.isCreated = false;
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "[INFO] [rx][attached] key:" << key() << "id:" << id();
        #endif
    } else { //--- pipe not exist, viewRx created
        if(!getControlBlockDataPtr())
            return;
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        TControlBlock::initRxView(getControlBlockView());
        mId = 0;
        params.isCreated = true;
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "[INFO] [rx][created] key:" << key() << "id:" << id();
        #endif
        mLastError = mStatus = IP_QPIPE_LIB::Ok;
    }
    //---
    mNotifier.setKeyPipeId(id());
    mControlBlockCache = getControlBlockView(); // not quarded
    params.pipeId   = id();
    params.pipeInfo = mControlBlockCache;
    mNotifier.start();
}

//------------------------------------------------------------------------------
TPipeViewRx::~TPipeViewRx()
{
    if(mControlBlockData && (id() != -1)) {
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        TControlBlock& controlBlockView = getControlBlockView();
        controlBlockView.rxReady[mId] = 0;
    }
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [TPipeViewRx destructor] key:" << key() << "id:" << id();
    #endif
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

