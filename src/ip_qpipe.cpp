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
    printf("--- pipe attached ---\n");
    printf("txReady: %1d\n",controlBlock.txReady);
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        printf("rxReady[%1d]: %1d\n",k,controlBlock.rxReady[k]);
    }
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
TPipeView::TPipeView(const QString& key) :
                                          mControlBlock(key +QString("_control")),
                                          mControlBlockData(0),
                                          mStatus(IP_QPIPE_LIB::NotInit),
                                          mLastError(IP_QPIPE_LIB::NotInit),
                                          mControlBlockCache()
{
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        //qDebug() << "[INFO] [TPipeView constructor]   key:" << key;
    #endif
}

//------------------------------------------------------------------------------
TPipeView::~TPipeView()
{
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        //qDebug() << "[INFO] [TPipeView destructor]    control block key:" << mControlBlock.key();
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
TPipeViewTx::TPipeViewTx(const QString& key, uint32_t chunkSize, uint32_t chunkNum) : TPipeView(key)
{
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        mSem[k] = 0;
    }

    if(!mControlBlock.create(sizeof(TPipeView::TControlBlock),QSharedMemory::ReadWrite)) {
        if(mControlBlock.error() != QSharedMemory::AlreadyExists) {
            #if defined(IP_QPIPE_PRINT_DEBUG_ERROR)
                qDebug() << "[ERROR] [TPipeViewTx constructor] at QSharedMemory::create";
            #endif
            mLastError = mStatus = IP_QPIPE_LIB::CreateError;
            return;
        }
        if(!attachControlBlock())
            return;
        if(!getControlBlockDataPtr())
            return;
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        /*DEBUG*/ TControlBlock::printInfo(getControlBlockView());
        if((mLastError = mStatus = TControlBlock::attachTxView(getControlBlockView(),chunkSize, chunkNum)) != IP_QPIPE_LIB::Ok)
            return;
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "[INFO] [tx][attached]" << key;
        #endif
    } else {
        if(!getControlBlockDataPtr())
            return;
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        TControlBlock::initTxView(getControlBlockView(),chunkSize, chunkNum);
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "[INFO] [tx][created]" << key;
        #endif
        mLastError = mStatus = IP_QPIPE_LIB::Ok;
    }

    //---
    for(auto k = 0; k < TPipeView::TControlBlock::MaxRxNum; ++k) {
        mSem[k] = new QSystemSemaphore((key + QString::number(k)),0,QSystemSemaphore::Create);
    }

    //---
    mControlBlockCache = getControlBlockView(); // not quarded
    notifyRx(mControlBlockCache);

    /*TEST*/ qDebug() << "is_standard_layout<TPipeView::TControlBlock>" << std::is_standard_layout<TPipeView::TControlBlock>::value;
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
        qDebug() << "[INFO] [TPipeViewTx destructor]" << mControlBlock.key();
    #endif
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
void TPipeViewRxNotifier::run()
{
    while(!mExit) {
        mSem.acquire();
        if(mExit)
            return;
        qDebug() << "slon 1" << QThread::currentThreadId();
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TPipeViewRx::TPipeViewRx(const QString& key, IP_QPIPE_LIB::TPipeInfo* pipeInfo) : TPipeView(key),
                                               mId(-1),
                                               mNotifier(key)
{
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
        /*DEBUG*/ TControlBlock::printInfo(getControlBlockView());
        if((mLastError = mStatus = TControlBlock::attachRxView(getControlBlockView(),mId)) != IP_QPIPE_LIB::Ok)
            return;
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "[INFO] [rx][attached]" << key << mId;
        #endif
    } else {
        if(!getControlBlockDataPtr())
            return;
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        TControlBlock::initRxView(getControlBlockView());
        mId = 0;
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "[INFO] [rx][created]" << key << mId;
        #endif
        mLastError = mStatus = IP_QPIPE_LIB::Ok;
    }
    //---
    mNotifier.setKey(mId);
    mControlBlockCache = getControlBlockView(); // not quarded
    if(pipeInfo) {
        *pipeInfo = mControlBlockCache;
    }
    mNotifier.start();
}

//------------------------------------------------------------------------------
TPipeViewRx::~TPipeViewRx()
{
    if(mControlBlockData && (mId != -1)) {
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        TControlBlock& controlBlockView = getControlBlockView();
        controlBlockView.rxReady[mId] = 0;
    }
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [TPipeViewRx destructor]" << mControlBlock.key() << mId;
    #endif
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
TPipeView* TPipeViewPool::getPipeView(const QString& key, TPipeViewPoolMap& pool)
{
    auto pipeView = pool.find(key);
    return (pipeView == pool.end()) ? 0 : pipeView->second;
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewPool::createPipeViewTx(const QString& key, uint32_t chunkSize, uint32_t chunkNum)
{
    if(isPipeViewTxExist(key)) {
        return IP_QPIPE_LIB::PipeExistError;
    }

    TPipeViewTx* pipeView = new TPipeViewTx(key,chunkSize,chunkNum);
    if(!pipeView->isPipeOk()) {
        IP_QPIPE_LIB::TStatus err = pipeView->error();
        delete pipeView;
        return err;
    }
    txPool().insert(std::pair<QString,TPipeView*>(key,pipeView));
    return IP_QPIPE_LIB::Ok;
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeViewPool::createPipeViewRx(const QString& key, IP_QPIPE_LIB::TPipeInfo* pipeInfo)
{
    if(isPipeViewRxExist(key)) {
        return IP_QPIPE_LIB::PipeExistError;
    }

    TPipeViewRx* pipeView = new TPipeViewRx(key, pipeInfo);
    if(!pipeView->isPipeOk()) {
        IP_QPIPE_LIB::TStatus err = pipeView->error();
        delete pipeView;
        return err;
    }
    rxPool().insert(std::pair<QString,TPipeView*>(key,pipeView));
    return IP_QPIPE_LIB::Ok;
}

