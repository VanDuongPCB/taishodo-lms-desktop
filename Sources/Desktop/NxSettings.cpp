#include "NxSettings.h"
#include "QCoreApplication"
#include "QDir"
#include "QFile"
#include "QFileInfo"
#include "QJsonDocument"
#include "QJsonArray"
#include "QJsonObject"

#include "NxFileManager.h"
#include "NxDatabase.h"
#include "NxEvent.h"

namespace
{
    const QString appRegPath = QString( "HKEY_CURRENT_USER\\Software\\Laser Marking System" );
}

HxRegistrySetting::HxRegistrySetting() :
    QSettings( appRegPath, QSettings::NativeFormat ),
    m_regPath( appRegPath )
{
    m_defaultValues[ DatabaseFilePath ] = value( DatabaseFilePath, "D:/LMS/LMS.db" );
    m_defaultValues[ DataRootDir ] = value( DataRootDir, qApp->applicationDirPath() + "/data" );
    m_defaultValues[ IVProgramDir ] = value( IVProgramDir, qApp->applicationDirPath() + "/data/IV-PROGRAMS" );
    m_defaultValues[ ThemeRootDir ] = value( ThemeRootDir, qApp->applicationDirPath() + "/themes" );
    m_defaultValues[ ThemeSetting ] = value( ThemeSetting, "Default" );
    m_defaultValues[ AdminPassword ] = value( AdminPassword, "Laser" );
    m_defaultValues[ SuperPassword ] = value( SuperPassword, "Super123" );
    m_defaultValues[ LeaderPassword ] = value( LeaderPassword, "Leader123" );
    m_defaultValues[ LaserConnPort ] = value( LaserConnPort, "COM1" );
    m_defaultValues[ PLCConnIPAddress ] = value( PLCConnIPAddress, "192.168.0.11" );
    m_defaultValues[ PLCConnPort ] = value( PLCConnPort, 8501 );
    m_defaultValues[ RegCvWidth ] = value( RegCvWidth, "DM230" );
    m_defaultValues[ RegStopper ] = value( RegStopper, "MR30103" );
    m_defaultValues[ RegPassMode ] = value( RegPassMode, "MR30107" );
    m_defaultValues[ RegSoftwareReady ] = value( RegSoftwareReady, "MR30100" );
    m_defaultValues[ RegLaserTrigger ] = value( RegLaserTrigger, "MR30000" );
    m_defaultValues[ RegLaserTriggerConfirm ] = value( RegLaserTriggerConfirm, "MR30110" );
    m_defaultValues[ RegLaserStop ] = value( RegLaserStop, "MR30001" );
    m_defaultValues[ RegLaserStopConfirm ] = value( RegLaserStopConfirm, "MR30111" );
    m_defaultValues[ RegBarcodeHasData ] = value( RegBarcodeHasData, "MR30002" );
    m_defaultValues[ RegBarcodeConfirm ] = value( RegBarcodeConfirm, "MR30111" );
    m_defaultValues[ RegBarcodeData ] = value( RegBarcodeData, "DM350" );
    m_defaultValues[ RegBarcodeOK ] = value( RegBarcodeOK, "MR30105" );
    m_defaultValues[ RegBarcodeNG ] = value( RegBarcodeNG, "MR30106" );
    m_defaultValues[ RegMarkingResult ] = value( RegMarkingResult, "MR30101" );
}

HxRegistrySetting& HxRegistrySetting::operator = ( const HxRegistrySetting& other)
{
    HxRegistrySetting setting;
    setting.m_dataLoaded = other.m_dataLoaded;
    setting.m_dataToSave = other.m_dataToSave;
    return setting;
}

bool HxRegistrySetting::Bool( QAnyStringView key )
{
    if ( m_dataLoaded.find( key ) != m_dataLoaded.end() )
        return m_dataLoaded[ key ].toBool();

    if ( contains( key ) )
    {
        m_dataLoaded[ key ] = value( key );
        return m_dataLoaded[ key ].toBool();
    }

    if ( m_defaultValues.find( key ) != m_defaultValues.end() )
        return m_defaultValues[ key ].toBool();

    return false;
}

int HxRegistrySetting::Int( QAnyStringView key )
{
    if ( m_dataLoaded.find( key ) != m_dataLoaded.end() )
        return m_dataLoaded[ key ].toInt();

    if ( contains( key ) )
    {
        m_dataLoaded[ key ] = value( key );
        return m_dataLoaded[ key ].toInt();
    }

    if ( m_defaultValues.find( key ) != m_defaultValues.end() )
        return m_defaultValues[ key ].toInt();

    return 0;
}

double HxRegistrySetting::Double( QAnyStringView key )
{
    if ( m_dataLoaded.find( key ) != m_dataLoaded.end() )
        return m_dataLoaded[ key ].toDouble();

    if ( contains( key ) )
    {
        m_dataLoaded[ key ] = value( key );
        return m_dataLoaded[ key ].toDouble();
    }

    if ( m_defaultValues.find( key ) != m_defaultValues.end() )
        return m_defaultValues[ key ].toDouble();

    return 0.0;
}

QString HxRegistrySetting::String( QAnyStringView key )
{
    if ( m_dataLoaded.find( key ) != m_dataLoaded.end() )
        return m_dataLoaded[ key ].toString();

    if ( contains( key ) )
    {
        m_dataLoaded[ key ] = value( key );
        return m_dataLoaded[ key ].toString();
    }

    if ( m_defaultValues.find( key ) != m_defaultValues.end() )
        return m_defaultValues[ key ].toString();

    return QString();
}

void HxRegistrySetting::Set( QAnyStringView key, const QVariant& value )
{
    m_dataToSave[ key ] = value;
}

void HxRegistrySetting::Clear()
{
    m_dataLoaded.clear();
    m_dataToSave.clear();
}

void HxRegistrySetting::Load()
{
    m_dataLoaded.clear();
    auto keys = allKeys();
    for ( auto& key : keys )
    {
        m_dataLoaded[ key ] = value( key );
    }
}

void HxRegistrySetting::Save()
{
    for ( auto &[key, value] : m_dataToSave )
    {
        setValue( key, value );
    }
}

