#include "NxMainWindow.h"
#include "QDebug"
#include "XDebug.h"

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
    a.setApplicationName( "LMS" );
    // Thử dùng style chuẩn của hệ điều hành (Windows: "windowsvista"/"windows11",
    // Ubuntu: GTK3 qua platform theme). Trước đây ép Fusion cho mọi nền tảng.
    // a.setStyle( "fusion" );

    // Chế độ debug: bật khi có <thư mục exe>/debug/lms_debug.json (đọc một lần,
    // xem XDebug). Log debug.log nằm cùng thư mục đó.
    XDebug::initialize( { QCoreApplication::applicationDirPath() + "/debug" } );
    XDebug::installMessageHandler();
    if ( XDebug::isEnabled() )
    {
        qInfo() << "LMS debug mode ON, file:" << XDebug::filePath();
        if ( !XDebug::lastError().isEmpty() )
            qWarning() << "lms_debug.json:" << XDebug::lastError();
    }

    const QString serverName = "LMS-LOCAL-SERVER";
    if ( IsRunning( serverName ) )
    {
        qInfo() << "LMS: another instance is running, exiting";
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
        qInfo() << "LMS: not licensed, register dialog closed, exiting";
        return 0;
    }

    NxDatabase::addDatabase( "QSQLITE", "SQLITE" );

    NxMainWindow w;
    w.show();
    int code = a.exec();
    qInfo() << "LMS: main window closed, exit code" << code;
    return code;
}
