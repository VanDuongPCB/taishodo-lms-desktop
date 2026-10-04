#include "NxDatabase.h"
#include "QFile"
#include "QObject"
#include "QJsonObject"
#include "QApplication"

#include "NxMessage.h"
#include "NxEvent.h"

NxDatabase::NxDatabase() : QSqlDatabase() 
{}

NxDatabase::NxDatabase( const QSqlDatabase& other ): QSqlDatabase(other)
{ }

NxDatabase::NxDatabase( const QString& fileDB ) : QSqlDatabase()
{
    setDatabaseName( fileDB );
}

bool NxDatabase::CheckDatabaseFileExisting( const QString& file )
{
    bool exist = QFile::exists( file );
    if ( !exist )
    {
        HxMsgError( QObject::tr( "Không tìm thấy file cơ sở dữ liệu!\n"
                                 "%1\n"
                                 "Hãy kiểm tra cài đặt hoặc thực hiện migrate!" )
                    .arg( file ),
                    QObject::tr( "Lỗi cơ sở dữ liệu" ) );
        QJsonObject obj;
        obj.insert( "time", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );
        obj.insert( "where", "Database" );
        obj.insert( "message", QString( "Không tìm thấy database: %1" ).arg( file ) );
        qApp->postEvent( qApp, new NxEvent( NxEvent::eDatabaseError, obj ) );
    }
    return exist;
}