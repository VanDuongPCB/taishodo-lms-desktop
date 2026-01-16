#pragma once
#include "NxDefines.h"
#include "NxLOT.h"
#include "NxModel.h"
#include "NxDesign.h"
#include "NxException.h"
#include "NxSettings.h"

struct HxLog
{
    QString Time;
    QString Serial;
    QString LOT;
    QString Model;
    QStringList items;
};

using HxLogArray = std::vector<HxLog>;

class NxLogger : private QObject
{
    Q_OBJECT
public:
    NxLogger();
    ReturnCode CheckSerialExisting( const QString& serial );
    ReturnCode SavePrint( NxLOTPtr pLOT, NxModelPtr pModel, NxDesignPtr pDesign, std::map<int, QString>& blockdata );
    ReturnCode SaveError( const QString& time, const QString& where, const QString& message );
    ReturnCode SaveBarcode( const QString& code );

    ReturnCode Get( QDate fromDate, QDate toDate, HxLogArray& items );
    ReturnCode Get( const QString& serial, HxLogArray& items );

    void Export( HxLogArray& items, QDate fromDate, QDate toDate );
    void ReLoadSetting();
private:
    HxRegistrySetting m_settings;
};


NxLogger* Logger();