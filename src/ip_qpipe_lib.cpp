//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define IP_QPIPE_LIB_EXPORT

#include "ip_qpipe_lib.h"
#include "ip_qpipe.h"

namespace IP_QPIPE_LIB
{
    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus createPipeViewTx(const char* key, uint32_t chunkSize, uint32_t chunkNum)
    {
        return TPipeViewPool::createPipeViewTx(QString::fromLocal8Bit(key), chunkSize, chunkNum);
    }

    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus createPipeViewRx(const char* key, IP_QPIPE_LIB::TPipeInfo* pipeInfo)
    {
        return TPipeViewPool::createPipeViewRx(QString::fromLocal8Bit(key), pipeInfo);
    }
}
