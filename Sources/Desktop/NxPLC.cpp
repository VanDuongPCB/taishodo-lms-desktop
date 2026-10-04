#include "NxPLC.h"
#include "QString"
#include "QApplication"

#include "NxDefines.h"
#include "NxTcpSocket.h"
#include "NxSettings.h"
#include "NxMessage.h"
#include "NxEvent.h"
#include "NxException.h"

NxPLC::NxPLC()
{
    m_settings.Load();
}

bool NxPLC::SetEnable( bool en )
{
#ifdef DEBUG_MODE
    return true;
#endif

    QString val = en ? "1" : "0";
    QString cmd = "WR " +  m_settings.String( RegSoftwareReady ) + " " + val + "\r";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "PLC" );
        throw ex;
    }
    return fb == "OK";
}

bool NxPLC::SetMarkResult( bool status )
{
#ifdef DEBUG_MODE
    return true;
#endif

    QString val = status == true ? "1 0" : "0 1";
    QString cmd = "WRS " + m_settings.String( RegMarkingResult ) + " 2 " + val + "\r";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "PLC" );
        throw ex;
    }
    return fb == "OK";
}

bool NxPLC::SetCvWidth( double width )
{
#ifdef DEBUG_MODE
    return true;
#endif

    int val = width * 100;
    QString cmd = "WR " + m_settings.String( RegCvWidth ) + " " + QString::number( val ) + "\r";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "PLC" );
        throw ex;
    }
    return fb == "OK";
}

bool NxPLC::SetTransferMode( bool on )
{
#ifdef DEBUG_MODE
    return true;
#endif

    QString cmd = "WR " + m_settings.String( RegPassMode ) + " " + QString::number( on ) + "\r";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "PLC" );
        throw ex;
    }
    return fb == "OK";
}

bool NxPLC::SetStopper( int stopper )
{
#ifdef DEBUG_MODE
    return true;
#endif

    QString val = stopper == 1 ? "1 0" : "0 1";
    QString cmd = "WRS " + m_settings.String( RegStopper ) + " 2 " + val + "\r";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "PLC" );
        throw ex;
    }
    return fb == "OK";
}

bool NxPLC::IsHasTrigger()
{
#ifdef DEBUG_MODE
    return true;
#endif

    QString cmd = "RD " + m_settings.String( RegLaserTrigger ) + "\r";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "PLC" );
        throw ex;
    }
    return fb == "1";
}

bool NxPLC::ConfirmTrigger()
{
#ifdef DEBUG_MODE
    return true;
#endif

    QString cmd = "WR " + m_settings.String( RegLaserTriggerConfirm ) + " 1\r\n";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "PLC" );
        throw ex;
    }
    return fb == "OK";
}

bool NxPLC::SetCompleteBit()
{
#ifdef DEBUG_MODE
    return true;
#endif

    QString cmd = "WR MR30115 1\r\n";
    QString fb;
    try
    {
        NxTcpSocket sock( m_settings.String( PLCConnIPAddress ), m_settings.Int( PLCConnPort ) );
        sock.Connect();
        sock.WriteLine( cmd );
        fb = sock.ReadLine().trimmed();
    }
    catch ( NxException ex )
    {
        ex.SetWhere( "PLC" );
        throw ex;
    }
    return fb == "OK";
}

void NxPLC::ReLoadSetting()
{
    m_settings.Load();
}

NxPLC* PLC()
{
    static NxPLC instance;
    return &instance;
}
