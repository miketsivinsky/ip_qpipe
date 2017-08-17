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
class T_IP_PipeView
{
    public:
        T_IP_PipeView(const QString& key);
        virtual ~T_IP_PipeView() = 0;
        bool isPipeOk() const { return mStatus == IP_QPIPE_LIB::Ok; }
        IP_QPIPE_LIB::TStatus error() const { return mLastError; }
        virtual bool transferBuf(uint8_t* buf, uint32_t& bufSize) = 0;
        QString pipeKey() const { return mSharedMemory.key(); }

    protected:
        class TLock
        {
            private:
                T_IP_PipeView& mOwner;
                bool           mLocked;

            public:
                TLock(T_IP_PipeView& owner) : mOwner(owner) {
                    mLocked = mOwner.mSharedMemory.lock();
                    if(!mLocked) {
                        mOwner.mSharedMemory.detach();
                        mOwner.mStatus = IP_QPIPE_LIB::LockError;
                    }
                }
                ~TLock() { mOwner.mSharedMemory.unlock(); }
                bool isLocked() const { return mLocked; }
        };

        static const size_t PageSize = 4096; // check at Linux

        //---
        struct TPipeControlBlock
        {
            uint32_t rdyTx;
            uint32_t rdyRx;
            uint32_t writeIdx;
            uint32_t chunkFilled;
            uint32_t chunkSize;
            uint32_t chunkNum;
            uint8_t  reserved[PageSize-6*sizeof(uint32_t)];
        };

        //---
        typedef uint64_t TTransferNumber;
        struct TChunkHeader
        {
            uint32_t        transferLen;
            TTransferNumber transferNum;
        };

        //---
        static const uint32_t PipeControlBlockSize  = sizeof(TPipeControlBlock);
        static const uint32_t ChunkHeaderSize       = sizeof(TChunkHeader);

        //---
        TPipeControlBlock& pipeControlBlock() { return *(reinterpret_cast<TPipeControlBlock*>(mData)); }
        void* getChunkHeaderPtr(uint32_t idx) const { return static_cast<uint8_t*>(mData) + PipeControlBlockSize + idx*(ChunkHeaderSize + mChunkSize); }
        void* getChunkDataPtr(uint32_t idx) const { return static_cast<uint8_t*>(getChunkHeaderPtr(idx)) + ChunkHeaderSize; }
        TChunkHeader& chunkHeader(uint32_t idx) { return *(static_cast<TChunkHeader*>(getChunkHeaderPtr(idx))); }

        //---
        QSharedMemory         mSharedMemory;
        uint32_t              mChunkSize;
        uint32_t              mChunkNum;
        void*                 mData;
        IP_QPIPE_LIB::TStatus mStatus;
        IP_QPIPE_LIB::TStatus mLastError;
        TTransferNumber       mTransferNumber;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class T_IP_PipeViewTx : public T_IP_PipeView
{
    public:
        T_IP_PipeViewTx(const QString& key, uint32_t chunkSize, uint32_t chunkNum);
        ~T_IP_PipeViewTx();
        bool transferBuf(uint8_t* buf, uint32_t& bufSize);

    protected:
        void advanceWriteIdx(TPipeControlBlock& pipeControl);
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class T_IP_PipeViewPool
{
    public:
        static bool isPipeViewTxExist(const QString& key) { return (getPipeView(key,txPool()) != 0); }
        static IP_QPIPE_LIB::TStatus createPipeViewTx(const QString& key, uint32_t chunkSize, uint32_t chunkNum);

    private:
        typedef std::map<QString,T_IP_PipeView*> T_IP_PipeViewPoolMap;

        static T_IP_PipeViewPool& instance() {
            static T_IP_PipeViewPool pool;
            return pool;
        }

        static T_IP_PipeViewPoolMap& txPool() { return instance().mTxPool; }

        T_IP_PipeViewPool() : mTxPool() { }
        ~T_IP_PipeViewPool();
        static T_IP_PipeView* getPipeView(const QString& key, T_IP_PipeViewPoolMap& pool);

        T_IP_PipeViewPoolMap mTxPool;
};

#endif /* IP_QPIPE_H */

