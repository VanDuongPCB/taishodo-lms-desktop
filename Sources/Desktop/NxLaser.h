#pragma once
#include "QString"
#include "QMap"
#include "QObject"

#include "NxPosition.h"
#include "NxDesign.h"
#include "NxStopper.h"
#include "NxSettings.h"

class NxLaser : public QObject
{
public:
    NxLaser();
    bool SetProgram( const QString& name );
    bool SetupBlockData( const QString& program, std::map<int, QString> dataMap );
    bool SetupPosition( const QString& program, NxPosition pos, NxDesignPtr pDesign, NxStopperPtr pStopper );
    bool Burn();
    void ReLoadSetting();
private:
    HxRegistrySetting m_settings;
    QString SendData( const QString& data, int timeout = 10000 );
};

NxLaser* Laser();
