#pragma once
#include "QString"
#include "QMap"
#include "QObject"
#include "NxException.h"

class NxSystemReport : public QObject
{
    Q_OBJECT

public:
    NxSystemReport();
    void Report( NxException ex );

signals:
    void reported( NxException ex );
};

NxSystemReport* GetSystemReport();