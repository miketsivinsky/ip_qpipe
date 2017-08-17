//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include <QDebug>

#include "ip_qpipe.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
T_IP_PipeView::T_IP_PipeView(const QString& key) :
                                                   mSharedMemory(key),
                                                   mChunkSize(0),
                                                   mChunkNum(0),
                                                   mStatus(IP_QPIPE_LIB::NotInit),
                                                   mLastError(IP_QPIPE_LIB::NotInit),
                                                   mTransferNumber(0)
{
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [T_IP_PipeView constructor]   key:" << key;
    #endif
}

//------------------------------------------------------------------------------
T_IP_PipeView::~T_IP_PipeView()
{
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [T_IP_PipeView destructor]    key:" << mSharedMemory.key();
    #endif
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
T_IP_PipeViewTx::T_IP_PipeViewTx(const QString& key, uint32_t chunkSize, uint32_t chunkNum) : T_IP_PipeView(key)
{
    mChunkSize = chunkSize;
    mChunkNum  = chunkNum;

    bool isCreatedBefore;

    //---
    if((isCreatedBefore = mSharedMemory.isAttached()) == false) {
        if(!mSharedMemory.create(PipeControlBlockSize + mChunkNum*(ChunkHeaderSize + mChunkSize),QSharedMemory::ReadWrite)) {
            #if defined(IP_QPIPE_PRINT_DEBUG_ERROR)
                qDebug() << "[ERROR] [T_IP_PipeViewTx constructor] at QSharedMemory::create";
            #endif
            mLastError = mStatus = IP_QPIPE_LIB::CreateError;
            return;
        }
    }

    //---
    if(!(mData = mSharedMemory.data())) {
        #if defined(IP_QPIPE_PRINT_DEBUG_ERROR)
            qDebug() << "[ERROR] [T_IP_PipeViewTx constructor] at QSharedMemory::data";
        #endif
        mSharedMemory.detach();
        mLastError = mStatus = IP_QPIPE_LIB::DataAccessError;
        return;
    }

    //---
    TPipeControlBlock& pipeControl = pipeControlBlock();
    {
        TLock lock(*this);
        if(!lock.isLocked()) {
            return;
        }

        if(isCreatedBefore) {
            if(pipeControl.rdyTx) {
                mSharedMemory.detach();
                mLastError = mStatus = IP_QPIPE_LIB::TxPipeExistError;
                return;
            }
            if((pipeControl.chunkNum != mChunkNum) || (pipeControl.chunkSize != mChunkSize)) {
                mSharedMemory.detach();
                mLastError = mStatus = IP_QPIPE_LIB::PipeParamError;
                return;
            }
        } else {
            pipeControl.chunkNum    = mChunkNum;
            pipeControl.chunkSize   = mChunkSize;
        }
        pipeControl.writeIdx    = 0;
        pipeControl.chunkFilled = 0;
        pipeControl.rdyTx       = 1;
    }

    //---
    mLastError = mStatus = IP_QPIPE_LIB::Ok;
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [T_IP_PipeViewTx constructor] key:" << key << "chunkSize:" << chunkSize << "chunkNum:" << chunkNum;
        qDebug() << "size:" << mSharedMemory.size();
    #endif
}

//------------------------------------------------------------------------------
T_IP_PipeViewTx::~T_IP_PipeViewTx()
{
    TPipeControlBlock& pipeControl = pipeControlBlock();
    {
        TLock lock(*this);
        if(lock.isLocked()) {
            pipeControl.rdyTx = 0;
        } else {
            #if defined(IP_QPIPE_PRINT_DEBUG_ERROR)
                qDebug() << "[ERROR] [T_IP_PipeViewTx destructor] lock error";
            #endif
        }
    }
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [T_IP_PipeViewTx destructor]  key:" << mSharedMemory.key();
    #endif
}

//------------------------------------------------------------------------------
bool T_IP_PipeViewTx::transferBuf(uint8_t* buf, uint32_t& bufSize)
{
    mLastError = IP_QPIPE_LIB::Ok;
    if(!isPipeOk()) {
        mLastError = mStatus;
        return false;
    }

    if(bufSize > mChunkSize) {
        mLastError = IP_QPIPE_LIB::FuncParamError;
        return false;
    }

    //---
    {
        TLock lock(*this);
        if(!lock.isLocked()) {
            mLastError = IP_QPIPE_LIB::LockError;
            return false;
        }
        TPipeControlBlock& pipeControl = pipeControlBlock();
        chunkHeader(pipeControl.writeIdx).transferLen = bufSize;
        chunkHeader(pipeControl.writeIdx).transferNum = mTransferNumber++;
        std::memcpy(getChunkDataPtr(pipeControl.writeIdx),buf,bufSize);
        advanceWriteIdx(pipeControl);
    }

    return true;
}

//------------------------------------------------------------------------------
void T_IP_PipeViewTx::advanceWriteIdx(TPipeControlBlock& pipeControl)
{
    pipeControl.writeIdx = (pipeControl.writeIdx == (mChunkNum-1)) ? 0 : (pipeControl.writeIdx + 1);
    if(pipeControl.chunkFilled < mChunkNum) {
        ++pipeControl.chunkFilled;
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
T_IP_PipeViewPool::~T_IP_PipeViewPool()
{
    for(auto pipeViewTx = txPool().begin(); pipeViewTx != txPool().end(); ++pipeViewTx) {
        #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
            qDebug() << "[INFO] [T_IP_PipeViewPool destructor] tx pipe with key:" << pipeViewTx->first << "will be deleted";
        #endif
        delete pipeViewTx->second;
    }
}

//------------------------------------------------------------------------------
T_IP_PipeView* T_IP_PipeViewPool::getPipeView(const QString& key, T_IP_PipeViewPoolMap& pool)
{
    auto pipeView = pool.find(key);
    return (pipeView == pool.end()) ? 0 : pipeView->second;
}

//------------------------------------------------------------------------------
IP_QPIPE_LIB::TStatus T_IP_PipeViewPool::createPipeViewTx(const QString& key, uint32_t chunkSize, uint32_t chunkNum)
{
    if(isPipeViewTxExist(key)) {
        return IP_QPIPE_LIB::TxPipeExistError;
    }

    T_IP_PipeViewTx* pipeView = new T_IP_PipeViewTx(key,chunkSize,chunkNum);
    if(!pipeView->isPipeOk()) {
        IP_QPIPE_LIB::TStatus err = pipeView->error();
        delete pipeView;
        return err;
    }
    txPool().insert(std::pair<QString,T_IP_PipeView*>(key,pipeView));
    return IP_QPIPE_LIB::Ok;
}
