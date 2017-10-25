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
        TSharedMemory(const QString &key, QObject *parent = Q_NULLPTR) : QSharedMemory(key, parent) {}
        bool isLockedByMe() { return d_func()->lockedByMe; }
        bool lock();
        bool unlock();
        //bool attach(QSharedMemory::AccessMode mode);

    protected:
        inline QSharedMemoryPrivate* d_func() { return reinterpret_cast<QSharedMemoryPrivate*>(qGetPtrHelper(d_ptr)); }
};


#endif /* SHARED_MEM_H */

