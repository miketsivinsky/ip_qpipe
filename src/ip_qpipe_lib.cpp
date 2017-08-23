//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define IP_QPIPE_LIB_EXPORT

#include "ip_qpipe_lib.h"
#include "ip_qpipe.h"

namespace IP_QPIPE_LIB
{
    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus createPipeViewTx(TPipeTxParams& params)
    {
        return TPipeViewPool::createPipeViewTx(params);
    }

    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus createPipeViewRx(const char* key, IP_QPIPE_LIB::TPipeInfo* pipeInfo)
    {
        return TPipeViewPool::createPipeViewRx(QString::fromLocal8Bit(key), pipeInfo);
    }
}
