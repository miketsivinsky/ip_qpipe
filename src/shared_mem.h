//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if !defined(SHARED_MEM_H)
#define SHARED_MEM_H

#include <QSharedMemory>
#include "qsharedmemory_p.h"
#include <QDebug>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TSharedMemory : public QSharedMemory
{
    public:
        TSharedMemory(unsigned pipeKey, const QString &key, QObject *parent = Q_NULLPTR) :
                                                                                           QSharedMemory(key, parent),
                                                                                           mPipeKey(pipeKey)
                                                                                           {}
        bool isLockedByMe() { return d_func()->lockedByMe; }
        bool lock(unsigned pipeKey = 0);
        bool unlock(unsigned pipeKey = 0);
        //bool attach(QSharedMemory::AccessMode mode);

    protected:
        unsigned mPipeKey;

        inline QSharedMemoryPrivate* d_func() { return reinterpret_cast<QSharedMemoryPrivate*>(qGetPtrHelper(d_ptr)); }
};


#endif /* SHARED_MEM_H */

