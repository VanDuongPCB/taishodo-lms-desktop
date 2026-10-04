#pragma once
#include "QString"
#include <QMap>
#include "QObject"
#include "NxException.h"

class NxSystemError : public QObject
{
    Q_OBJECT

public:
    NxSystemError();
    void ErrorReport( NxException ex );

signals:
    void Reported( NxException ex );

public:
    static NxSystemError* Instance();
};
