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
        TSharedMemory(QObject *parent = Q_NULLPTR) :
                                                                       QSharedMemory(parent),
                                                                       mKeyId(QString())
                                                                       {}
        TSharedMemory(const QString &keyId, QObject *parent = Q_NULLPTR) :
                                                                       QSharedMemory(keyId, parent),
                                                                       mKeyId(keyId)
                                                                       {}
        bool isLockedByMe() { return d_func()->lockedByMe; }
        bool lock(const QString& keyId = QString());
        bool unlock(const QString& keyId = QString());
        //bool attach(QSharedMemory::AccessMode mode);

    protected:
        QString mKeyId;

        inline QSharedMemoryPrivate* d_func() { return reinterpret_cast<QSharedMemoryPrivate*>(qGetPtrHelper(d_ptr)); }
};


#endif /* SHARED_MEM_H */

