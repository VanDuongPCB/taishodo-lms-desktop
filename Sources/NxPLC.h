#pragma once
#include "QObject"
#include "NxSettings.h"


class NxPLC : private QObject
{
    Q_OBJECT
public:
    NxPLC();
    bool SetEnable( bool en );
    bool SetMarkResult( bool status );
    bool SetCvWidth( double width );
    bool SetTransferMode( bool on );
    bool SetStopper( int stopper );
    bool IsHasTrigger();
    bool ConfirmTrigger();
    bool SetCompleteBit();
    void ReLoadSetting();
private:
    HxRegistrySetting m_settings;
};

NxPLC* PLC();