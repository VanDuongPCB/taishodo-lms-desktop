
#include "NxTcpSocket.h"
#include "NxException.h"
#include "NxSystemReport.h"

NxTcpSocket::NxTcpSocket( QObject* parent ) : QTcpSocket{ parent }
{

}

NxTcpSocket::NxTcpSocket( const QString& ip, int port, QObject* parent )
{
    this->m_ip = ip;
    this->m_port = port;
}

bool NxTcpSocket::Connect( int timeout )
{
    connectToHost( m_ip, m_port );
    if ( !waitForConnected( timeout ) )
    {
        QString msg = QString(
            "Kết nối tới plc hết hạn!\n"
            "IP: %1\n"
            "Port: %2" ).arg( m_ip ).arg( m_port );
        throw NxException( msg );
    }
    return true;
}

bool NxTcpSocket::WriteLine( const QString& data, int timeout )
{
    std::string s = data.toStdString();
    this->write( s.data(), s.length() );
    if ( !waitForBytesWritten( timeout ) )
    {
        QString msg = QString(
            "Gửi dữ liệu đến plc hết hạn !\n"
            "IP: %1\n"
            "Port: %2" ).arg( m_ip ).arg( m_port );
        throw NxException( msg );
    }
    return true;
}

QString NxTcpSocket::ReadLine( int timeout )
{
    if ( !waitForReadyRead( timeout ) )
    {
        QString msg = QString(
            "Plc không phản hồi !\n"
            "IP: %1\n"
            "Port: %2" ).arg( m_ip ).arg( m_port );
        throw NxException( msg );
    }
    char buffer[ 4096 ]{ 0 };
    this->read( buffer, 4095 );
    return QString::fromStdString( buffer ).trimmed();
}
