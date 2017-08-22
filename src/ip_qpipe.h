//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if !defined(IP_QPIPE_H)
#define IP_QPIPE_H

#include <cstdint>
#include <map>

#include <QThread>
#include <QSharedMemory>
#include <QSystemSemaphore>

#include "ip_qpipe_def.h"

#define IP_QPIPE_PRINT_DEBUG_INFO
#define IP_QPIPE_PRINT_DEBUG_ERROR

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeView
{
    public:
        TPipeView(const QString& key);
        virtual ~TPipeView();
        bool isPipeOk() const { return mStatus == IP_QPIPE_LIB::Ok; }
        IP_QPIPE_LIB::TStatus error() const { return mLastError; }

        //---
        static const int MaxRxNum = 4;

        struct TControlBlock
        {
            uint32_t chunkSize;
            uint32_t chunkNum;
            uint32_t txReady;
            uint32_t rxReady[MaxRxNum];

            static void printInfo(TControlBlock& controlBlock);
            static void initTxView(TControlBlock& controlBlock, uint32_t chunkSize, uint32_t chunkNum);
            static IP_QPIPE_LIB::TStatus attachTxView(TControlBlock& controlBlock, uint32_t chunkSize, uint32_t chunkNum);
            static void initRxView(TControlBlock& controlBlock);
            static IP_QPIPE_LIB::TStatus attachRxView(TControlBlock& controlBlock,int& rxId);
        };

   protected:
        //---
        class TLock
        {
            private:
                QSharedMemory& mSharedMem;
                bool           mLocked;

            public:
                TLock(QSharedMemory& sharedMem) : mSharedMem(sharedMem), mLocked(sharedMem.lock()) {}
                ~TLock() { mSharedMem.unlock(); }
                bool isLocked() const { return mLocked; }
        };

        //---
        TControlBlock& getControlBlockView() { return *(reinterpret_cast<TControlBlock*>(mControlBlockData)); }
        bool attachControlBlock();
        bool getControlBlockDataPtr();

        //---
        QSharedMemory         mControlBlock;
        void*                 mControlBlockData;
        IP_QPIPE_LIB::TStatus mStatus;
        IP_QPIPE_LIB::TStatus mLastError;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeViewTx : public TPipeView
{
    public:
        TPipeViewTx(const QString& key, uint32_t chunkSize, uint32_t chunkNum);
        ~TPipeViewTx();
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeViewRxNotifier : public QThread
{
    Q_OBJECT

    public:
        TPipeViewRxNotifier(const QString& semKey) : mExit(false), mSem(semKey,0,QSystemSemaphore::Create) {}
        ~TPipeViewRxNotifier() { mExit = true; mSem.release(); wait(WaitForFinish); }
        virtual void run() Q_DECL_OVERRIDE;
        void setKey(int rxId) {  mSem.setKey(mSem.key() + QString::number(rxId)); }

    protected:
        static const unsigned WaitForFinish = 1000;

        bool             mExit;
        QSystemSemaphore mSem;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeViewRx : public TPipeView
{
    public:
        TPipeViewRx(const QString& key);
        ~TPipeViewRx();

    protected:
        int                 mId;
        TPipeViewRxNotifier mNotifier;

};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeViewPool
{
    public:
        static bool isPipeViewTxExist(const QString& key) { return (getPipeView(key,txPool()) != 0); }
        static bool isPipeViewRxExist(const QString& key) { return (getPipeView(key,rxPool()) != 0); }
        static IP_QPIPE_LIB::TStatus createPipeViewTx(const QString& key, uint32_t chunkSize, uint32_t chunkNum);
        static IP_QPIPE_LIB::TStatus createPipeViewRx(const QString& key);

    private:
        typedef std::map<QString,TPipeView*> TPipeViewPoolMap;

        static TPipeViewPool& instance() {
            static TPipeViewPool pool;
            return pool;
        }

        static TPipeViewPoolMap& txPool() { return instance().mTxPool; }
        static TPipeViewPoolMap& rxPool() { return instance().mRxPool; }

        TPipeViewPool() : mTxPool(), mRxPool() { }
        ~TPipeViewPool();
        static TPipeView* getPipeView(const QString& key, TPipeViewPoolMap& pool);

        TPipeViewPoolMap mTxPool;
        TPipeViewPoolMap mRxPool;
};

#endif /* IP_QPIPE_H */

