#include "QCoreApplication"
#include "QTime"
#include "QDir"
#include "QFile"

#include "NxDefines.h"
#include "NxBarcode.h"
#include "NxSettings.h"
#include "NxTcpSocket.h"
#include "NxMessage.h"
#include "NxFileManager.h"
#include "NxEvent.h"
#include "NxException.h"

NxBarcode::NxBarcode() : QObject( nullptr )
{
    m_settings.Load();
}

bool NxBarcode::IsHasData()
{
#ifdef DEBUG_MODE
    return true;
#endif

    QString cmd = "RD " +m_settings.String( RegBarcodeHasData ) + "\r";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd, 5000 );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "Barcode" );
        throw ex;
    }
    return fb == "1";
}

bool NxBarcode::Clear()
{
#ifdef DEBUG_MODE
    return true;
#endif

    QString cmd = "WR " + m_settings.String( RegLaserTriggerConfirm ) + " 1\r";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd, 5000 );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "Barcode" );
        throw ex;
    }
    return fb == "OK";
}

QString NxBarcode::Read()
{
#ifdef DEBUG_MODE
    return "";
#endif

    QString cmd = "RDS " + m_settings.String( RegBarcodeData ) + " 40\r";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd, 5000 );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "Barcode" );
        throw ex;
    }

    QStringList codeItems = fb.split( ' ' );
    if ( codeItems.size() < 1 ) 
        return "";
    std::vector<wchar_t> buffer( codeItems.size() + 1, 0 );
    for ( int i = 0; i < codeItems.size(); i++ )
    {
        int val = codeItems[ i ].toInt( 0 );
        buffer[ i ] = ( ( val >> 8 ) | ( val << 8 ) ) & 0xFFFF;
    }

    char* ptr = ( char* )buffer.data();

    return QString::fromStdString( ptr );
}

bool NxBarcode::SendFeedback( bool status )
{
#ifdef DEBUG_MODE
    return true;
#endif

    QString val1 = status == 1 ? "1" : "0";
    QString val2 = status == 1 ? "0" : "1";
    QString cmd = "WRS " + m_settings.String( RegBarcodeOK ) + " 2 " + val1 + " " + val2 + "\r";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd, 2000 );
        fb = sock.readLine( 2000 );
        cmd = "WR " + m_settings.String( RegBarcodeConfirm ) + " 1\r";
        sock.WriteLine( cmd, 2000 );
        fb = sock.readLine( 2000 );
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "Barcode" );
        throw ex;
    }
    return fb == "OK";
}

void NxBarcode::ReLoadSetting()
{
    m_settings.Load();
}

NxBarcode* Barcode()
{
    static NxBarcode instance;
    return &instance;
}
