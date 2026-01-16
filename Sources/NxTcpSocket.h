#pragma once
#include "QTcpSocket"

class NxTcpSocket : public QTcpSocket
{
public:
    explicit NxTcpSocket( QObject* parent = nullptr );
    explicit NxTcpSocket( const QString& ip, int port, QObject* parent = nullptr );
    bool Connect( int timeout = 5000 );
    bool WriteLine( const QString& data, int timeout = 2000 );
    QString ReadLine( int timeout = 2000 );
private:
    QString m_ip;
    int m_port = 0;
};

