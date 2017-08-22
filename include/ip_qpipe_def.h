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
        PipeExistError      = -3,  // tx/rx pipe exist, attempt to open another one
        CreateError         = -4,  // create error
        DataAccessError     = -5,  // data access error
        AttachError         = -6,  // attach error
        AttachTxParamsError = -7,  // previously created pipe params != current attached pipe params (actual only for 'writer')
        AttachTxExistError  = -8,  // attemt to attach as 'writer' when another one writer exist now
        AttachRxExistError  = -9,  // attemt to attach as 'reader' but number of existed 'readers' over limit
    } TStatus;
}

#ifdef __cplusplus
    }
#endif

#endif /* IP_QPIPE_DEF_H */

