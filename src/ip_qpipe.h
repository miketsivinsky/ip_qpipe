//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if !defined(IP_QPIPE_H)
#define IP_QPIPE_H

#include <cstdint>
#include <map>

#include <QThread>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QSemaphore>

#include <QDebug>

#include "tqueue.h"
#include "ip_qpipe_def.h"

#include "shared_mem.h"

#define IP_QPIPE_PRINT_DEBUG_INFO
#define IP_QPIPE_PRINT_DEBUG_ERROR

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeView;
class TPipeViewRx;

class TPipeViewRxNotifier : public QThread
{
    Q_OBJECT

    public:
        //---
        static QString genKey(const TPipeView& pipeView, int rxId = -1);

        //---
        TPipeViewRxNotifier(TPipeViewRx& pipeViewRx);
        ~TPipeViewRxNotifier() {
            if(!mExit)
                stop();
        }
        virtual void run() Q_DECL_OVERRIDE;
        void setKeyPipeId(int rxId);
        bool stop() {
            mExit = true;
            mGblSem.release();
            bool status = wait(WaitForFinish);
            return status;
        }

    protected:
        static const unsigned WaitForFinish = 1000;

        bool             mExit;
        QSystemSemaphore mGblSem;
        TPipeViewRx&     mPipeViewRx;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeView
{
    public:
        TPipeView(unsigned pipeKey);
        virtual ~TPipeView();
        bool isPipeOk() const { return mStatus == IP_QPIPE_LIB::Ok; }
        IP_QPIPE_LIB::TStatus error() const { return mLastError; }
        unsigned key() const { return mKey; }
        bool isRxPresent() { return TControlBlock::isRxPresent(getControlBlockView()); }

        //--- TControlBlock
        struct TControlBlock : public IP_QPIPE_LIB::TPipeInfo
        {
            //---
            uint32_t txBufIdx;
            uint32_t txGblIdx;
            uint32_t txBufEmpty;

            //---
            static void printInfo(TControlBlock& controlBlock);
            static void initTxView(TControlBlock& controlBlock, uint32_t chunkSize, uint32_t chunkNum);
            static IP_QPIPE_LIB::TStatus attachTxView(TControlBlock& controlBlock, uint32_t chunkSize, uint32_t chunkNum);
            static void initRxView(TControlBlock& controlBlock);
            static IP_QPIPE_LIB::TStatus attachRxView(TControlBlock& controlBlock,int& rxId);
            static bool isRxPresent(TControlBlock& controlBlock);

            //---
            TControlBlock();
            TControlBlock& operator=(const TControlBlock& right);
        };

        //--- TChunkHeader
        struct TChunkHeader
        {
            uint32_t chunkLen;
        };

   protected:

        //---
        struct TChunk
        {
            TChunkHeader* chunkHeader;
            uint8_t*      chunkData;
        };

        //---
        class TLock
        {
            private:
                TSharedMemory& mSharedMem;
                bool           mLocked;

                bool           mEnaTrace;
                int            mTraceId;

            public:
                TLock(TSharedMemory& sharedMem, bool enaTrace = false, int traceId = 0) :
                    mSharedMem(sharedMem),
                    mLocked(sharedMem.lock()),
                    mEnaTrace(enaTrace),
                    mTraceId(traceId)
                {
                    if(mEnaTrace) {
                        qDebug() << "[DEBUG] TPipeView::TLock locked. key:" << mSharedMem.key() << "traceId:" << mTraceId;
                    }
                }
                ~TLock()
                {
                    mSharedMem.unlock();
                    if(mEnaTrace) {
                        qDebug() << "[DEBUG] TPipeView::TLock unlocked. key:" << mSharedMem.key() << "traceId:" << mTraceId;
                    }
                }
                bool isLocked() const { return mLocked; }
        };

        //---
        TControlBlock& getControlBlockView() { return *(reinterpret_cast<TControlBlock*>(mControlBlockData)); }
        bool attachControlBlock();
        bool getControlBlockDataPtr();
        bool attachDataBlock(QSharedMemory::AccessMode accessMode = QSharedMemory::ReadWrite);
        bool getDataBlockDataPtr();
        TChunk getChunk(uint32_t idx);

        //---
        TSharedMemory         mControlBlock;
        void*                 mControlBlockData;

        //---
        TSharedMemory         mDataBlock;
        void*                 mDataBlockData;

        IP_QPIPE_LIB::TStatus mStatus;
        IP_QPIPE_LIB::TStatus mLastError;
        TControlBlock         mControlBlockCache;
        unsigned              mKey;
};

bool operator==(const TPipeView::TControlBlock& left, const TPipeView::TControlBlock& right);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeViewTx : public TPipeView
{
    public:
        TPipeViewTx(IP_QPIPE_LIB::TPipeTxParams& params);
        ~TPipeViewTx();
        IP_QPIPE_LIB::TStatus sendData(IP_QPIPE_LIB::TPipeTxTransfer& txTransfer);
        IP_QPIPE_LIB::TStatus sendData(IP_QPIPE_LIB::TPipeTxTransferFuncObj& txTransfer);

    protected:
        unsigned notifyRx(const TPipeView::TControlBlock& controlBlock);
        bool activatePipe(IP_QPIPE_LIB::TPipeTxParams& params);
        bool activateDataBlock(IP_QPIPE_LIB::TPipeTxParams& params);

        QSystemSemaphore* mSem[TPipeView::TControlBlock::MaxRxNum];
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeViewRx : public TPipeView
{
    friend class TPipeViewRxNotifier;

    public:
        TPipeViewRx(IP_QPIPE_LIB::TPipeRxParams& params);
        ~TPipeViewRx();
        int id() const { return mId; }
        IP_QPIPE_LIB::TStatus readData(IP_QPIPE_LIB::TPipeRxTransfer& rxTransfer, int timeout);
        IP_QPIPE_LIB::TStatus readData(IP_QPIPE_LIB::TPipeRxTransferFuncObj& rxTransfer, int timeout);

    protected:
        IP_QPIPE_LIB::TTxEvent whatTxEvent();
        bool activatePipe(IP_QPIPE_LIB::TPipeRxParams& params);
        bool isRxSemSignalEna();
        uint32_t computeRxBufIdx(uint32_t idxNormDelta) const;
        void syncRxGblIdx(uint32_t offset = 0);

        bool dataBlockOn();
        bool dataBlockOff();
        bool dataBlockOnNoLock();
        bool dataBlockOffNoLock(bool semReset = true);

        int                               mId;
        TPipeViewRxNotifier               mNotifier;
        IP_QPIPE_LIB::PipeRxNotifyFunc    mNotifyFunc;
        uint32_t                          mRxGblIdx;
        QSemaphore                        mRxSem;
        TQtMutexGuard                     mInstanceGuard;
        TQtMutexGuard                     mDataBlockGuard;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeViewPool
{
    public:
        static bool isPipeViewTxExist(unsigned key) { return (getPipeView(key,txPool()) != 0); }
        static bool isPipeViewRxExist(unsigned key) { return (getPipeView(key,rxPool()) != 0); }
        static IP_QPIPE_LIB::TStatus createPipeViewTx(IP_QPIPE_LIB::TPipeTxParams& params);
        static IP_QPIPE_LIB::TStatus createPipeViewRx(IP_QPIPE_LIB::TPipeRxParams& params);
        static IP_QPIPE_LIB::TStatus deletePipeViewTx(unsigned pipeKey) { return deletePipeView(pipeKey, txPool()); }
        static IP_QPIPE_LIB::TStatus deletePipeViewRx(unsigned pipeKey) { return deletePipeView(pipeKey, rxPool()); }
        static IP_QPIPE_LIB::TStatus sendData(IP_QPIPE_LIB::TPipeTxTransfer& txTransfer);
        static IP_QPIPE_LIB::TStatus sendData(IP_QPIPE_LIB::TPipeTxTransferFuncObj& txTransfer);
        static IP_QPIPE_LIB::TStatus readData(IP_QPIPE_LIB::TPipeRxTransfer& rxTransfer, int timeout);
        static IP_QPIPE_LIB::TStatus readData(IP_QPIPE_LIB::TPipeRxTransferFuncObj& rxTransfer, int timeout);

    private:
        typedef std::map<unsigned,TPipeView*> TPipeViewPoolMap;

        static TPipeViewPool& instance() {
            static TPipeViewPool pool;
            return pool;
        }

        static TPipeViewPoolMap& txPool() { return instance().mTxPool; }
        static TPipeViewPoolMap& rxPool() { return instance().mRxPool; }

        TPipeViewPool() : mTxPool(), mRxPool() { }
        ~TPipeViewPool();
        static TPipeView* getPipeView(unsigned key, TPipeViewPoolMap& pool);
        static IP_QPIPE_LIB::TStatus deletePipeView(unsigned pipeKey, TPipeViewPoolMap& pool);

        TPipeViewPoolMap mTxPool;
        TPipeViewPoolMap mRxPool;
};

#endif /* IP_QPIPE_H */

