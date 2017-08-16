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
                                                   mChunkNum(0)
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

    if(!mSharedMemory.create(mChunkSize*mChunkNum + sizeof(T_IP_PipeView::TPipeControlBlock),QSharedMemory::ReadWrite)) {
        #if defined(IP_QPIPE_PRINT_DEBUG_ERROR)
            qDebug() << "[ERROR] [T_IP_PipeViewTx constructor] at QSharedMemory::create";
        #endif
    }

    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [T_IP_PipeViewTx constructor] key:" << key << "chunkSize:" << chunkSize << "chunkNum:" << chunkNum;
        qDebug() << "size:" << mSharedMemory.size();
    #endif
}

//------------------------------------------------------------------------------
T_IP_PipeViewTx::~T_IP_PipeViewTx()
{
    #if defined(IP_QPIPE_PRINT_DEBUG_INFO)
        qDebug() << "[INFO] [T_IP_PipeViewTx destructor]  key:" << mSharedMemory.key();
    #endif
}

