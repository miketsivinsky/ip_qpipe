//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include <QtGlobal>
#include <QDebug>

//------------------------------------------------------------------------------

#include "ip_qpipe.h"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void TPipeView::TControlBlock::initTxView(TPipeView::TControlBlock& controlBlock, uint32_t chunkSize, uint32_t chunkNum)
{
    controlBlock.chunkSize = chunkSize;
    controlBlock.chunkNum  = chunkNum;
    controlBlock.txReady   = 1;
    for(auto k = 0; k < TPipeView::MaxRxNum; ++k) {
        controlBlock.rxReady[k] = 0;
    }
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus TPipeView::TControlBlock::attachTxView(TControlBlock& controlBlock, uint32_t chunkSize, uint32_t chunkNum)
{
    if((controlBlock.chunkSize != chunkSize) || (controlBlock.chunkNum != chunkNum))
        return IP_QPIPE_LIB::AttachTxParamsError;
    if(controlBlock.txReady)
        return IP_QPIPE_LIB::AttachTxExistError;
    controlBlock.txReady = 1;
    return IP_QPIPE_LIB::Ok;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TPipeView::TPipeView(const QString& key) :
                                          mControlBlock(key +QString("_control")),
                                          mControlBlockData(0),
                                          mStatus(IP_QPIPE_LIB::NotInit),
                                          mLastError(IP_QPIPE_LIB::NotInit)
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
    }

    //---
    mLastError = mStatus = IP_QPIPE_LIB::Ok;
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        //qDebug() << "[INFO] [TPipeViewTx constructor] key:" << key << "chunkSize:" << chunkSize << "chunkNum:" << chunkNum;
        //qDebug() << "control block size:" << mControlBlock.size();
    #endif
}

//------------------------------------------------------------------------------
TPipeViewTx::~TPipeViewTx()
{
    {
        TLock lockControlBlock(mControlBlock); // TODO: check - locked or not
        TControlBlock& controlBlockView = getControlBlockView();
        controlBlockView.txReady = 0;
    }
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [TPipeViewTx destructor]  control block key:" << mControlBlock.key();
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
        return IP_QPIPE_LIB::TxPipeExistError;
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


