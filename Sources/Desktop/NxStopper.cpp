#include "NxStopper.h"
#include "QCoreApplication"
#include "QDir"
#include "QFile"
#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"

#include "NxFileManager.h"
#include "NxDatabase.h"
#include "NxEvent.h"

NxStopperManager::NxStopperManager() : QObject( nullptr )
{
    m_settings.Load();
}

NxStopperPtr NxStopperManager::Create()
{
    return std::make_shared<NxStopper>();
}

NxStopperPtr NxStopperManager::GetStopper( int index )
{
    NxStopperPtr pStopper;

    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return pStopper;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
        return pStopper;
    NxQuery query( db );
    QString cmd = QString( "SELECT Number,X,Y FROM Stoppers WHERE Number='%1'" ).arg( index );
    if ( !query.exec( cmd ) )
    {
        db.close();
        return pStopper;
    }

    while ( query.next() )
    {
        if ( !pStopper )
        {
            pStopper = Create();
            pStopper->x = query.value( 1 ).toDouble();
            pStopper->y = query.value( 2 ).toDouble();
        }
    }
    db.close();
    return pStopper;
}

NxStopperPtrMap NxStopperManager::GetStoppers()
{
    NxStopperPtrMap map;

    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return map;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
        return map;
    NxQuery query( db );
    QString cmd = QString( " SELECT Number,X,Y FROM Stoppers" );
    if ( !query.exec( cmd ) )
    {
        db.close();
        return map;
    }
    while ( query.next() )
    {
        NxStopperPtr pStopper = Create();
        int index = query.value( 0 ).toInt();
        pStopper->x = query.value( 1 ).toDouble();
        pStopper->y = query.value( 2 ).toDouble();
        map[ index ] = pStopper;
    }
    db.close();
    return map;
}

ReturnCode NxStopperManager::Save( int index, NxStopperPtr pStopper )
{
    if ( !pStopper )
        return RtDataNull;

    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return RtDBFileNotFound;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
        return RtDBOpenFailed;

    NxQuery query( db );
    QString cmd = QString( "UPDATE Stoppers "
                           "SET X = '%1',Y = '%2' "
                           "WHERE Number = '%3'" )
        .arg( pStopper->x )
        .arg( pStopper->y )
        .arg( index );
    if ( !query.exec( cmd ) )
    {
        db.close();
        return RtDBQueryFailed;
    }
    db.close();
    return RtNormal;
}

void NxStopperManager::ReLoadSetting()
{
    m_settings.Load();
}

NxStopperManager* StopperManager()
{
    static NxStopperManager instance;
    return &instance;
}