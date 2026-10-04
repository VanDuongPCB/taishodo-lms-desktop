#include "NxLicense.h"
#include "QSysInfo"
#include "QCoreApplication"
#include "QFile"
#include "NxMessage.h"
#include "NxSettings.h"

namespace
{
    const int s_majorVersion = BUILD_MAJOR_VERSION;
    const int s_minorVersion = BUILD_MINOR_VERSION;
    const int s_buildVersion = BUILD_PATCH_VERSION;
    const int s_appTypeID = 1000;
    const QString s_kernel = "A8FEA5ED-EC06-4225-9B50-A58FF6C74860";
}

QString HxLicensing::ReadKey() const
{
    HxRegistrySetting setting;
    QString keyVersion = QString( "License/%1.%2" ).arg( s_majorVersion ).arg( s_minorVersion );
    return setting.String( keyVersion );
}

void HxLicensing::WriteKey( const QString& key ) const
{
    HxRegistrySetting setting;
    QString keyVersion = QString( "License/%1.%2" ).arg( s_majorVersion ).arg( s_minorVersion );
    setting.Clear();
    setting.Set( keyVersion, key );
    setting.Save();
}

bool HxLicensing::IsRegistered()
{
    QString keyRef = KeyFromId( ID() );
    QString keyStore = ReadKey();
    return keyStore == keyRef;
}

QString HxLicensing::ID()
{
    return QSysInfo::machineUniqueId().toUpper();
}

QString HxLicensing::KeyFromId( const QString& id ) const
{
    const std::string c = "0123456789ABCDEF";
    int numOfChars = c.length();
    std::string kernel = s_kernel.toStdString();
    std::string sid = id.toStdString();
    int size = sid.length();
    std::vector<char> buff;
    for ( int i = 0; i < size; i++ )
    {
        if ( kernel[ i ] == '-' )
        {
            buff.push_back( sid[ i ] );
            continue;
        }

        int val = abs( ( int )sid[ i ] * ( int )kernel[ i ] * i + s_appTypeID );
        val = val % numOfChars;
        buff.push_back( c[ val ] );
    }
    buff.push_back( 0 );
    return QString::fromStdString( buff.data() );
}

bool HxLicensing::RegisterKey( const QString& key )
{
    QString keyRef = KeyFromId( ID() );
    if ( keyRef == key )
    {
        WriteKey( key );
        return true;
    }
    else
    {
        return false;
    }
}

QString HxLicensing::GetVersion()
{
    return QString( "%1.%2.%3" )
        .arg( s_majorVersion )
        .arg( s_minorVersion )
        .arg( s_buildVersion );
}

HxLicensing* Licensing()
{
    static HxLicensing instance;
    return &instance;
}
