#pragma once
#include "QObject"
#include "QString"
#include "NxSettings.h"


class NxBarcode : private QObject
{
    Q_OBJECT
public:
    NxBarcode();
    bool IsHasData();
    bool Clear();
    QString Read();
    bool SendFeedback( bool status );
    void ReLoadSetting();
private:
    HxRegistrySetting m_settings;
};

NxBarcode* Barcode();