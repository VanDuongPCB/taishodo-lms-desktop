#include "NxMainWindow.h"

#include "QApplication"
#include "QLocalSocket"
#include "QLocalServer"
#include "QStyleFactory"

#include "NxLicense.h"
#include "NxDatabase.h"
#include "NxRegisterDialog.h"
#include "NxMessage.h"


static bool IsRunning( const QString& key )
{
    QLocalSocket socket;
    socket.connectToServer( key );
    if ( socket.waitForConnected( 100 ) )
    {
        // Đã có instance khác đang chạy
        return true; 
    }

    // Xóa nếu có server cũ bị kẹt
    QLocalServer::removeServer( key ); 
    return false;
}

int main( int argc, char* argv[] )
{
    QApplication a( argc, argv );
    a.setStyle( "fusion" );

    const QString serverName = "LMS-LOCAL-SERVER";
    if ( IsRunning( serverName ) )
    {
        HxMsgError( QObject::tr( "Phần mềm đang chạy.\n"
                                 "Không thể mở thêm!" ) );
        return 0;
    }

    QLocalServer server;
    server.listen( serverName );

    bool bIsLicensed = Licensing()->IsRegistered();

    if ( !bIsLicensed )
    {
        NxRegisterDialog licenseDialog;
        if ( licenseDialog.exec() )
            bIsLicensed = true;
    }

    if ( !bIsLicensed )
    {
        return 0;
    }

    NxDatabase::addDatabase( "QSQLITE", "SQLITE" );

    NxMainWindow w;
    w.show();
    int code = a.exec();
    return code;
}
