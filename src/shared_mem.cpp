//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


#include "shared_mem.h"

//#define SHARED_MEM_THERAD_LOCK
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool TSharedMemory::lock(const QString& keyId)
{
    if(keyId == key()) {
        qDebug() << "I: lock entry key:" << key();
    }

    QSharedMemoryPrivate* d = d_func();

    if (d->lockedByMe) {
        qDebug() << "W: QSharedMemory::lock: already locked, key:" << key();
        #if !defined(SHARED_MEM_THERAD_LOCK)
            return true;
        #endif
    }
    if (d->systemSemaphore.acquire()) {
        d->lockedByMe = true;
        return true;
    }
    QString function = QLatin1String("QSharedMemory::lock");
    d->errorString = QSharedMemory::tr("%1: unable to lock").arg(function);
    d->error = QSharedMemory::LockError;
    return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool TSharedMemory::unlock(const QString& keyId)
{
    if(keyId == key()) {
        qDebug() << "I: unlock entry key:" << key();
    }

    QSharedMemoryPrivate* d = d_func();

    #if !defined(SHARED_MEM_THERAD_LOCK)
        if (!d->lockedByMe)
            return false;
    #endif
    d->lockedByMe = false;
    if (d->systemSemaphore.release())
        return true;
    QString function = QLatin1String("QSharedMemory::unlock");
    d->errorString = QSharedMemory::tr("%1: unable to unlock").arg(function);
    d->error = QSharedMemory::LockError;
    return false;
}


#if 0

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool TSharedMemory::attach(QSharedMemory::AccessMode mode)
{
    QSharedMemoryPrivate* d = d_func();

    if (isAttached() || !d->initKey())
        return false;
#ifndef QT_NO_SYSTEMSEMAPHORE
    QSharedMemoryLocker lock(this);
    if (!d->key.isNull() && !d->tryLocker(&lock, QLatin1String("QSharedMemory::attach")))
        return false;
#endif

    if (isAttached() || !d->handle())
        return false;

    return d->attach(mode);
}

#endif
