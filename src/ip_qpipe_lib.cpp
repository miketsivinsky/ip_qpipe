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
        return T_IP_PipeViewPool::createPipeViewTx(QString::fromLocal8Bit(key), chunkSize, chunkNum);
    }

}
