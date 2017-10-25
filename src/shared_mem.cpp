//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


#include "shared_mem.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool TSharedMemory::lock(unsigned pipeKey)
{
    if(pipeKey == mPipeKey) {
        qDebug() << "I: lock entry pipeKey:" << pipeKey;
    }

    QSharedMemoryPrivate* d = d_func();

    if (d->lockedByMe) {
        qWarning("QSharedMemory::lock: already locked");
        return true;
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
bool TSharedMemory::unlock(unsigned pipeKey)
{
    if(pipeKey == mPipeKey) {
        qDebug() << "I: unlock entry pipeKey:" << pipeKey;
    }

    QSharedMemoryPrivate* d = d_func();

    if (!d->lockedByMe)
        return false;
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
