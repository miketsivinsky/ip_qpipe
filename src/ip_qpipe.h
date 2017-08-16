//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if !defined(IP_QPIPE_H)
#define IP_QPIPE_H

#include <cstdint>
#include <QSharedMemory>

#define IP_QPIPE_PRINT_DEBUG_INFO
#define IP_QPIPE_PRINT_DEBUG_ERROR

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class T_IP_PipeView
{
    public:
        T_IP_PipeView(const QString& key);
        virtual ~T_IP_PipeView() = 0;

    protected:
        static const size_t PageSize = 4096; // check at Linux

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

        QSharedMemory mSharedMemory;
        uint32_t      mChunkSize;
        uint32_t      mChunkNum;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class T_IP_PipeViewTx : public T_IP_PipeView
{
    public:
        T_IP_PipeViewTx(const QString& key, uint32_t chunkSize, uint32_t chunkNum);
        ~T_IP_PipeViewTx();

    protected:
};

#endif /* IP_QPIPE_H */

