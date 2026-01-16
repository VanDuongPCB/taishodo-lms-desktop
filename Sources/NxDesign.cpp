#include "NxDesign.h"
#include "QDir"
#include "QFile"
#include "QFileInfo"
#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"
#include "QCoreApplication"

#include "NxFileManager.h"
#include "NxDatabase.h"
#include "NxEvent.h"

NxDesign::NxDesign()
{

}

NxDesign::~NxDesign()
{

}

QString NxDesign::Name() const
{
    return m_name;
}

double NxDesign::Width() const
{
    return m_width;
}

double NxDesign::Height() const
{
    return m_height;
}

NxBlock NxDesign::Block( int index )
{
    return m_blocks[ index ];
}

std::map<int, NxBlock> NxDesign::Blocks()
{
    return m_blocks;
}

int NxDesign::IndexOfBlockCode()
{
    if ( m_blocks.empty() )
        return -1;

    int rindex = 1;
    for ( auto& [index, block] : m_blocks )
    {
        if ( block.isCode )
        {
            rindex = index;
            break;
        }
    }
    return rindex;
}


void NxDesign::SetName( const QString& name )
{
    Modify( m_name, name, eNew );
}

void NxDesign::SetWidth( double value )
{
    Modify( m_width, value, eSize );
}

void NxDesign::SetHeight( double value )
{
    Modify( m_height, value, eSize );
}

void NxDesign::SetBlock( int index, const NxBlock& block )
{
    auto it = m_blocks.find( index );
    if ( it == m_blocks.end() || it->second.data != block.data || it->second.isCode != block.isCode || it->second.textLen != block.textLen )
    {
        m_blocks[ index ] = block;
        SetModified( eBlock );
    }
}

NxDesignManager::NxDesignManager() :QObject( nullptr )
{
    m_settings.Load();
}

NxDesignPtr NxDesignManager::Create()
{
    return std::make_shared<NxDesign>();
}

NxDesignPtr NxDesignManager::GetDesign( const QString& name )
{
    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return nullptr;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
        return nullptr;
    NxQuery query( db );

    QString cmd = QString( "SELECT Name,Width,Height "
                           "FROM Designs "
                           "WHERE Name='%1'" )
        .arg( name );

    if ( !query.exec( cmd ) )
        return nullptr;

    NxDesignPtr pDesign;
    while ( query.next() )
    {
        if ( !pDesign )
        {
            pDesign = Create();
            pDesign->SetName( query.value( 0 ).toString() );
            pDesign->SetWidth( query.value( 1 ).toDouble() );
            pDesign->SetHeight( query.value( 2 ).toDouble() );
        }
    }

    if ( pDesign )
    {
        cmd = QString( "SELECT Number,Format,Length,IsCode "
                       "FROM DesignBlocks "
                       "WHERE Name='%1'" )
            .arg( name );
        if ( query.exec( cmd ) )
        {
            while ( query.next() )
            {
                int index = query.value( 0 ).toInt();
                if ( index < 1 )
                    continue;
                NxBlock block;
                block.data = query.value( 1 ).toString();
                block.textLen = query.value( 2 ).toInt();
                block.isCode = query.value( 3 ).toInt() > 0;
                pDesign->SetBlock( index, block );
            }
        }
        pDesign->ClearModified();
    }

    db.close();

    return pDesign;
}

NxDesignPtrMap NxDesignManager::GetDesigns()
{
    NxDesignPtrMap map;
    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return map;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
        return map;
    NxQuery query( db );

    QString cmd = QString( "SELECT Name,Width,Height FROM Designs" );

    if ( !query.exec( cmd ) )
        return map;

    while ( query.next() )
    {
        NxDesignPtr pDesign = Create();
        pDesign->SetName( query.value( 0 ).toString() );
        pDesign->SetWidth( query.value( 1 ).toDouble() );
        pDesign->SetHeight( query.value( 2 ).toDouble() );
        pDesign->ClearModified();
        map[ pDesign->Name() ] = pDesign;
    }
    db.close();

    return map;
}

ReturnCode NxDesignManager::Save( NxDesignPtr pDesign, bool bForce )
{
    if ( !pDesign )
        return RtDataNull;

    if ( !bForce && !pDesign->IsMofified() )
        return RtDataNoChanges;

    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return RtDBFileNotFound;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
    {
        return RtDBOpenFailed;
    }
    NxQuery query( db );
    QString cmd;
    if ( pDesign->IsMofified( NxDesign::eNew ) )
    {
        cmd = QString( "INSERT INTO Designs "
                       "(Name,Width,Height) "
                       "VALUES('%1',%2,%3)" )
            .arg( pDesign->Name() )
            .arg( pDesign->Width() )
            .arg( pDesign->Height() );
        if ( !query.exec( cmd ) )
        {

            qDebug() << query.lastError().text();
            db.close();
            return RtDBInsertFailed;
        }
    }
    else
    {
        cmd = QString( "UPDATE Designs "
                       "SET Width = '%2',Height = '%3' "
                       "WHERE Name = '%1'" )
            .arg( pDesign->Name() )
            .arg( pDesign->Width() )
            .arg( pDesign->Height() );
        query.exec( cmd );
    }

    if ( pDesign->IsMofified( NxDesign::eNew | NxDesign::eBlock ) )
    {
        cmd = QString( "DELETE FROM DesignBlocks WHERE Name = '%1'" ).arg( pDesign->Name() );
        query.exec( cmd );

        auto blocks = pDesign->Blocks();
        for ( auto& [index, block] : blocks )
        {
            cmd = QString( "INSERT INTO DesignBlocks "
                           "(Name,Number,Format,Length,IsCode) "
                           "VALUES('%1',%2,'%3',%4,%5);" )
                .arg( pDesign->Name() )
                .arg( index )
                .arg( block.data )
                .arg( block.textLen )
                .arg( block.isCode );
            if ( !query.exec( cmd ) )
            {
                qDebug() << query.lastError().text() << " - " << cmd;
            }
        }
    }

    db.close();
    bool isNew = pDesign->IsMofified( NxDesign::eNew );
    pDesign->ClearModified();
    return RtNormal;
}

//
//void NxDesignManager::Migration( const QString& dir )
//{
//    for ( int i = 0; i < 2000; i++ )
//    {
//        QString name = QString::number( i ).rightJustified( 4, '0' );
//        QString designPath = dir + "/" + name + ".design";
//        QFile fileReader( designPath );
//        if ( !fileReader.open( QIODevice::ReadOnly ) )
//            continue;
//
//        QByteArray json = fileReader.readAll();
//        fileReader.close();
//        QJsonDocument doc = QJsonDocument::fromJson( json );
//        QJsonObject obj = doc.object();
//
//        NxDesignPtr pDesign = Create();
//        pDesign->SetName( name );
//        pDesign->SetWidth( obj.value( "width" ).toDouble( 5 ) );
//        pDesign->SetHeight( obj.value( "height" ).toDouble( 5 ) );
//
//        QJsonArray arr = obj.value( "blocks" ).toArray();
//        for ( auto arrItem : arr )
//        {
//            QJsonObject objBlock = arrItem.toObject();
//            int index = objBlock.value( "index" ).toInt( 0 );
//            if ( index < 1 )
//                continue;
//
//            NxBlock block;
//            block.isCode = objBlock.value( "is-code" ).toBool( false );
//            block.data = objBlock.value( "data" ).toString();
//            block.textLen = objBlock.value( "text-length" ).toInt( 1 );
//            pDesign->SetBlock( index, block );
//        }
//
//        Save( pDesign );
//    }
//}

ReturnCode NxDesignManager::DeleteAll()
{
    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return RtDBFileNotFound;
    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
    {
        qDebug() << "Delete Design: " << db.lastError().text();
        return RtDBOpenFailed;
    }
    NxQuery query( db );
    QString cmd;

    cmd = QString( "DELETE FROM Designs" );
    if ( !query.exec( cmd ) )
    {
        qDebug() << "Delete Design: " << query.lastError().text();
        db.close();
        return RtDBQueryFailed;
    }

    cmd = QString( "DELETE FROM DesignParams" );
    if ( !query.exec( cmd ) )
    {
        qDebug() << "Delete Design: " << query.lastError().text();
        db.close();
        return RtDBQueryFailed;
    }

    cmd = QString( "DELETE FROM DesignBlocks" );
    if ( !query.exec( cmd ) )
    {
        qDebug() << "Delete Design: " << query.lastError().text();
        db.close();
        return RtDBQueryFailed;
    }

    db.close();
    return RtNormal;
}

void NxDesignManager::ReLoadSetting()
{
    m_settings.Load();
}

NxDesignManager* DesignManager()
{
    static NxDesignManager instance;
    return &instance;
}