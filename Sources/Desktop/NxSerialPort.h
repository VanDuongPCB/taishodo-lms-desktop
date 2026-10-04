#pragma once
#include "QSerialPort"



class NxSerialPort : public QSerialPort
{
public:
    explicit NxSerialPort( QObject* parent = nullptr );
    explicit NxSerialPort( const QString& port, QObject* parent = nullptr );
    explicit NxSerialPort( const QString& port, int baud, QObject* parent = nullptr );

    bool WriteLine( const QString& data, int timeout = 30000 );
    QString ReadLine( int timeout = 30000 );
};

