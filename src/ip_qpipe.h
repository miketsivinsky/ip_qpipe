//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if !defined(IP_QPIPE_H)
#define IP_QPIPE_H

#include <cstdint>
#include <map>

#include <QSharedMemory>

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
        static const unsigned MaxRxNum = 4;

        struct TControlBlock
        {
            uint32_t chunkSize;
            uint32_t chunkNum;
            uint32_t txReady;
            uint32_t rxReady[MaxRxNum];

            static void initTxView(TControlBlock& controlBlock, uint32_t chunkSize, uint32_t chunkNum);
            static IP_QPIPE_LIB::TStatus attachTxView(TControlBlock& controlBlock, uint32_t chunkSize, uint32_t chunkNum);
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

   protected:
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TPipeViewPool
{
    public:
        static bool isPipeViewTxExist(const QString& key) { return (getPipeView(key,txPool()) != 0); }
        static IP_QPIPE_LIB::TStatus createPipeViewTx(const QString& key, uint32_t chunkSize, uint32_t chunkNum);

    private:
        typedef std::map<QString,TPipeView*> TPipeViewPoolMap;

        static TPipeViewPool& instance() {
            static TPipeViewPool pool;
            return pool;
        }

        static TPipeViewPoolMap& txPool() { return instance().mTxPool; }

        TPipeViewPool() : mTxPool() { }
        ~TPipeViewPool();
        static TPipeView* getPipeView(const QString& key, TPipeViewPoolMap& pool);

        TPipeViewPoolMap mTxPool;
};

#endif /* IP_QPIPE_H */

