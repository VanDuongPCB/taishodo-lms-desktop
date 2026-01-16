#include "NxMigration.h"
#include "QApplication"
#include "QString"
#include "QFileInfo"
#include "QDir"
#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"
#include "QThread"

#include "NxLOT.h"
#include "NxModel.h"
#include "NxDesign.h"
#include "NxProfile.h"
#include "NxStopper.h"
#include "NxSettings.h"
#include "NxMessage.h"

void CheckAndCopyDB()
{
    HxRegistrySetting setting;
    QString databaseFile = setting.String( DatabaseFilePath );
    if ( QFile( databaseFile ).exists() )
    {
        QFileInfo fi( databaseFile );
        QString backupFile = fi.absoluteDir().absolutePath() + "/" + fi.baseName() + "_" + QDateTime::currentDateTime().toString("yyyyMMddHHmmss") + ".db";
        QFile::copy( databaseFile, backupFile );
    }
    else
    {
        QString defaultFile = qApp->applicationDirPath() + "/Default Files/LMS.db";
        QFile::copy( defaultFile, databaseFile );
    }
    QThread::msleep( 1000 );
}

void MigrateLOTs( const QString& dir )
{
    QString lotDir = dir;
    if ( !QDir().exists( dir ) )
    {
        HxMsgError( QObject::tr( "Thư mục dữ liệu LOT không tồn tại\n%1" ).arg( dir ), QObject::tr( "Migration LOT" ) );
        return;
    }

    CheckAndCopyDB();
    if ( LOTManager()->DeleteAll() != RtNormal )
    {
        HxMsgError( QObject::tr( "Không xóa được dữ liệu cũ!" ), QObject::tr( "Migration LOT" ) );
        return;
    }

    QFileInfoList fileInfos = QDir( lotDir ).entryInfoList( { "*.lot" } );

    std::set<QString> paramNames;

    for ( auto& fileInfo : fileInfos )
    {
        QString lotName = fileInfo.baseName().toUpper();
        QString filePath = dir + "/" + lotName + ".lot";

        QFileInfo fileInfo( filePath );
        QFile fileReader( filePath );
        if ( !fileReader.open( QIODevice::ReadOnly ) )
            continue;

        QDateTime lastWrite = fileInfo.lastModified();
        QByteArray json = fileReader.readAll();
        fileReader.close();

        qDebug() << "LOT: " << lotName;

        QJsonDocument doc = QJsonDocument::fromJson( json );
        QJsonObject obj = doc.object();

        NxLOTPtr pLOT = LOTManager()->Create();

        pLOT->SetName( fileInfo.baseName().toUpper() );
        pLOT->SetMACStart( obj.value( "mac-start" ).toString() );
        pLOT->SetMACEnd( obj.value( "mac-end" ).toString() );
        pLOT->SetCounterStart( obj.value( "counter-start" ).toString() );
        pLOT->SetQuantity( obj.value( "quantity" ).toInt() );
        pLOT->SetProgress( obj.value( "progress" ).toInt( 0 ) );
        pLOT->SetModel( obj.value( "model" ).toString() );
        pLOT->SetRePrint( obj.value( "is-re-print" ).toBool() );
        QJsonObject cmtObj = obj.value( "comments" ).toObject();
        QStringList keys = cmtObj.keys();
        for ( auto& key : keys )
        {
            pLOT->SetValue( key, cmtObj.value( key ).toString() );
            paramNames.insert( key.trimmed().toUpper() );
        }
        pLOT->Evaluate();
        
        ReturnCode code = LOTManager()->Save( pLOT );
        //if ( code != RtNormal )
        //{
        //    HxMsgError( QObject::tr( "Không cập nhật được LOT: %1\nMã lỗi: %2" ).arg( pLOT->Name() ).arg( code ), 
        //                QObject::tr( "Migration LOT" ) );
        //    return;
        //}
    }
    HxMsgInfo( QObject::tr( "Đã hoàn thành migrate LOT!" ),
                QObject::tr( "Migration LOT" ) );
}

void MigrateModels( const QString& dir )
{
    QString lotDir = dir;
    if ( !QDir().exists( dir ) )
    {
        HxMsgError( QObject::tr( "Thư mục dữ liệu Model không tồn tại\n%1" ).arg( dir ), QObject::tr( "Migration Model" ) );
        return;
    }

    CheckAndCopyDB();
    if ( ModelManager()->DeleteAll() != RtNormal )
    {
        HxMsgError( QObject::tr( "Không xóa được dữ liệu cũ!" ), QObject::tr( "Migration Model" ) );
        return;
    }

    QFileInfoList fileInfos = QDir( lotDir ).entryInfoList( { "*.model" } );

    std::set<QString> paramNames;

    for ( auto& fileInfo : fileInfos )
    {
        QString designName = fileInfo.baseName().toUpper();
        QString filePath = dir + "/" + designName + ".model";

        QFileInfo fileInfo( filePath );
        QFile fileReader( filePath );
        if ( !fileReader.open( QIODevice::ReadOnly ) )
            continue;

        QByteArray json = fileReader.readAll();
        fileReader.close();

        QJsonDocument doc = QJsonDocument::fromJson( json );
        QJsonObject obj = doc.object();

        NxModelPtr pModel = ModelManager()->Create();

        pModel->SetCode( obj.value( "code" ).toString() );
        pModel->SetName( designName );
        pModel->SetPrintLo( obj.value( "is-print-lo" ).toBool() );
        pModel->SetkNo( obj.value( "k-no" ).toString() );
        pModel->SetIVProgram( obj.value( "iv-program" ).toString() );
        pModel->SetDesign( obj.value( "design" ).toString() );
        pModel->SetCvWidth( obj.value( "cv-width" ).toDouble() );
        pModel->SetStopper( obj.value( "stopper" ).toInt( 1 ) );

        // markPositions;
        QJsonArray posArr = obj.value( "pos" ).toArray();
        int posIndex = 1;
        for ( auto posItem : posArr )
        {
            QJsonObject objPos = posItem.toObject();
            NxPosition pos;
            pos.index = objPos.value( "index" ).toInt();
            if ( pos.index <= 0 )
                pos.index = posIndex;
            pos.x = objPos.value( "x" ).toDouble( 0 );
            pos.y = objPos.value( "y" ).toDouble( 0 );
            pos.angle = objPos.value( "angle" ).toDouble( 0 );

            pModel->SetPosition( pos.index, pos );
            posIndex++;
        }

        // comments;
        QJsonObject commentObj = obj.value( "comments" ).toObject();
        QStringList keys = commentObj.keys();
        for ( auto& key : keys )
        {
            if ( key.startsWith( "LBLDIR" ) ) continue;
            if ( key.startsWith( "LBLPOSX" ) ) continue;
            if ( key.startsWith( "LBLPOSY" ) ) continue;
            if ( key.startsWith( "LBLSIZE" ) ) continue;
            if ( key.startsWith( "LBLTYPE" ) ) continue;
            pModel->SetComment( key, commentObj.value( key ).toString() );
        }

        ReturnCode code = ModelManager()->Save( pModel );
        //if ( code != RtNormal )
        //{
        //    HxMsgError( QObject::tr( "Không cập nhật được Model: %1\nMã lỗi: %2" ).arg( pModel->Name() ).arg( code ),
        //                QObject::tr( "Migration LOT" ) );
        //    return;
        //}
    }
    HxMsgInfo( QObject::tr( "Đã hoàn thành migrate Model!" ),
               QObject::tr( "Migration Model" ) );
}

void MigrateDesigns( const QString& dir )
{
    QString lotDir = dir;
    if ( !QDir().exists( dir ) )
    {
        HxMsgError( QObject::tr( "Thư mục dữ liệu Design không tồn tại\n%1" ).arg( dir ), QObject::tr( "Migration Design" ) );
        return;
    }

    CheckAndCopyDB();
    if ( DesignManager()->DeleteAll() != RtNormal )
    {
        HxMsgError( QObject::tr( "Không xóa được dữ liệu cũ!" ), QObject::tr( "Migration Design" ) );
        return;
    }

    for ( int i = 0; i < 2000; i++ )
    {
        QString name = QString::number( i ).rightJustified( 4, '0' );
        QString designPath = dir + "/" + name + ".design";
        QFile fileReader( designPath );
        if ( !fileReader.open( QIODevice::ReadOnly ) )
            continue;

        QByteArray json = fileReader.readAll();
        fileReader.close();
        QJsonDocument doc = QJsonDocument::fromJson( json );
        QJsonObject obj = doc.object();

        NxDesignPtr pDesign = DesignManager()->Create();
        pDesign->SetName( name );
        pDesign->SetWidth( obj.value( "width" ).toDouble( 5 ) );
        pDesign->SetHeight( obj.value( "height" ).toDouble( 5 ) );

        QJsonArray arr = obj.value( "blocks" ).toArray();
        for ( auto arrItem : arr )
        {
            QJsonObject objBlock = arrItem.toObject();
            int index = objBlock.value( "index" ).toInt( 0 );
            if ( index < 1 )
                continue;

            NxBlock block;
            block.isCode = objBlock.value( "is-code" ).toBool( false );
            block.data = objBlock.value( "data" ).toString();
            block.textLen = objBlock.value( "text-length" ).toInt( 1 );
            pDesign->SetBlock( index, block );
        }

        DesignManager()->Save( pDesign );
    }

    HxMsgInfo( QObject::tr( "Đã hoàn thành migrate Design!" ),
               QObject::tr( "Migration Design" ) );
}

void MigratePrintLogs( const QString& dir )
{
    CheckAndCopyDB();
}

void MigrateRePrintLogs( const QString& dir )
{
    CheckAndCopyDB();
}

void MigrateBarcodes( const QString& dir )
{
    CheckAndCopyDB();
}

void MigrateStoppers( const QString& file )
{
    if ( !QFile().exists( file ) )
    {
        HxMsgError( QObject::tr( "File stopper không tồn tại\n%1" ).arg( file ), QObject::tr( "Migration Stopper" ) );
        return;
    }

    CheckAndCopyDB();

    QFile reader( file );
    if ( !reader.open( QIODevice::ReadOnly ) )
        return;

    QByteArray json = reader.readAll();
    reader.close();
    QJsonDocument doc = QJsonDocument::fromJson( json );
    QJsonArray arr = doc.array();
    for ( auto arrItem : arr )
    {
        QJsonObject obj = arrItem.toObject();
        NxStopperPtr pStopper = StopperManager()->Create();
        int index = obj.value( "index" ).toInt( 0 );
        pStopper->x = obj.value( "x" ).toDouble( 0 );
        pStopper->y = obj.value( "y" ).toDouble( 0 );
        StopperManager()->Save( index, pStopper );
        qDebug() << "SAVE POINT " << index;
    }
    HxMsgInfo( QObject::tr( "Đã hoàn thành migrate Stopper!" ),
               QObject::tr( "Migration Stopper" ) );
}