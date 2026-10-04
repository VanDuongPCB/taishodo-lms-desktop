
#include "NxSerialPort.h"
#include "NxException.h"
#include "NxSystemReport.h"

NxSerialPort::NxSerialPort( QObject* parent ) : QSerialPort{ parent }
{

}

NxSerialPort::NxSerialPort( const QString& port, QObject* parent ) : QSerialPort{ parent }
{
    this->setPortName( port );
}

NxSerialPort::NxSerialPort( const QString& port, int baud, QObject* parent ) : QSerialPort{ parent }
{
    this->setPortName( port );
    this->setBaudRate( baud );
}

bool NxSerialPort::WriteLine( const QString& data, int timeout )
{
    std::string wdata = data.toStdString() + "\n";
    this->write( wdata.c_str() );
    if ( !waitForBytesWritten( timeout ) )
    {
        QStringList items = {
            "Không gửi được dữ liệu trên cổng serial !",
            "Cổng : " + portName()
        };
        throw NxException( items.join( "\n" ) );
    }
    return true;
}

QString NxSerialPort::ReadLine( int timeout )
{
    if ( !isOpen() )
    {
        QStringList items = {
            "Cổng dữ liệu đang đóng !",
            "Cổng : " + portName()
        };
        throw NxException( items.join( "\n" ) );
    }

    if ( !waitForReadyRead( timeout ) )
    {
        QStringList items = {
            "Không nhận được phản hồi trên cổng serial !",
            "Cổng : " + portName()
        };
        throw NxException( items.join( "\n" ) );
    }

    std::vector<char> buffer;
    char oneBuff[ 1 ];
    do
    {
        if ( bytesAvailable() )
        {
            read( oneBuff, 1 );
            if ( oneBuff[ 0 ] != 13 ) buffer.push_back( oneBuff[ 0 ] );
            else break;
        }
        else if ( !waitForReadyRead( 20 ) )
        {
            break;
        }
    } while ( true );
    close();

    buffer.push_back( 0 );
    QString data = QString::fromStdString( buffer.data() );
    return data;
}
