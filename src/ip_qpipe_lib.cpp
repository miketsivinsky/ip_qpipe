//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define IP_QPIPE_LIB_EXPORT

#include "ip_qpipe_lib.h"
#include "ip_qpipe.h"

namespace IP_QPIPE_LIB
{
    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus createPipeViewTx(IP_QPIPE_LIB::TPipeTxParams& params)
    {
        return TPipeViewPool::createPipeViewTx(params);
    }

    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus createPipeViewRx(IP_QPIPE_LIB::TPipeRxParams& params)
    {
        return TPipeViewPool::createPipeViewRx(params);
    }

    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus deletePipeViewTx(unsigned pipeKey)
    {
        return TPipeViewPool::deletePipeViewTx(pipeKey);
    }

    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus deletePipeViewRx(unsigned pipeKey)
    {
        return TPipeViewPool::deletePipeViewRx(pipeKey);
    }

    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus sendData(TPipeTxTransfer& txTransfer)
    {
        return TPipeViewPool::sendData(txTransfer);
    }

    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus sendDataFuncObj(TPipeTxTransferFuncObj& txTransfer)
    {
        return TPipeViewPool::sendData(txTransfer);
    }


    //------------------------------------------------------------------------------
    IP_QPIPE_DLL_API IP_QPIPE_LIB::TStatus readData(TPipeRxTransfer& rxTransfer, int timeout)
    {
        return TPipeViewPool::readData(rxTransfer, timeout);
    }


}
