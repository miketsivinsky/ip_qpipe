//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifndef IP_QPIPE_DEF_H
#define IP_QPIPE_DEF_H


#ifdef __cplusplus
    extern "C" {
#endif

namespace IP_QPIPE_LIB
{
    typedef enum
    {
        Ok                  =  0,  // all ok
        NotInit             = -1,  // not initialized
        CreateError         = -2,  // create error
        DataAccessError     = -3,  // data access error
        LockError           = -4,  // lock error
        TxPipeExistError    = -5,  // tx pipe exist, attempt to open another one
        PipeParamError      = -6,  // previously created pipe params != current pipe params
        FuncParamError      = -7   // bad function params
    } TStatus;
}

#ifdef __cplusplus
    }
#endif

#endif /* IP_QPIPE_DEF_H */

