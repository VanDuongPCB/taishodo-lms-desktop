#include "NxModel.h"
#include "QFileInfo"
#include "QJsonDocument"
#include "QJsonArray"
#include "QJsonObject"
#include "QStringList"
#include "QDir"
#include "QFile"
#include "QCoreApplication"

#include "NxFileManager.h"
#include "NxDatabase.h"
#include "NxEvent.h"

NxModel::NxModel() : NxObject()
{
    m_comments[ "FIX1" ] = "";
    m_comments[ "FIX2" ] = "";
    m_comments[ "FIX3" ] = "";
    m_comments[ "FIX4" ] = "";
    m_comments[ "FIX5" ] = "";
}

NxModel::~NxModel()
{

}

QString NxModel::Code() const
{
    return m_code;
}

QString NxModel::Name( bool bModifyMask ) const
{
    return m_name + ( ( bModifyMask && IsMofified() ) ? "*" : "" );
}

bool NxModel::IsPrintLo() const
{
    return m_bIsPrintLo;
}

QString NxModel::kNo() const
{
    return m_kNo;
}

QString NxModel::IVProgram() const
{
    return m_ivProgram;
}

QString NxModel::Design() const
{
    return m_design;
}

double NxModel::CvWidth() const
{
    return m_cvWidth;
}

int NxModel::Stopper() const
{
    return m_stopper;
}

std::map<int, NxPosition>& NxModel::Positions()
{
    return m_positions;
}

NxPosition NxModel::Position( int index ) const
{
    auto it = m_positions.find( index );
    if ( it != m_positions.end() )
        return it->second;
    return NxPosition();
}

std::map<QString, QString>& NxModel::Comments()
{
    return m_comments;
}

QString NxModel::Comment( const QString& name ) const
{
    auto it = m_comments.find( name );
    if ( it != m_comments.end() )
        return it->second;
    return QString();
}

QString NxModel::Value( QString paramName ) const
{
    if ( paramName == "CODE" ) return m_code;
    else if ( paramName == "NAME" ) return m_name;
    else if ( paramName == "NO" ) return m_kNo;
    else
    {
        auto it = m_comments.find( paramName );
        if ( it != m_comments.end() )
            return it->second;
        return "MODEL." + paramName;
    }
}

void NxModel::SetCode( const QString& value )
{
    Modify( m_code, value, eNew );
}

void NxModel::SetName( const QString& value )
{
    Modify( m_name, value, eInfo );
}

void NxModel::SetPrintLo( bool bIsEnable )
{
    Modify( m_bIsPrintLo, bIsEnable, eInfo );
}

void NxModel::SetkNo( const QString& value )
{
    Modify( m_kNo, value, eInfo );
}

void NxModel::SetIVProgram( const QString& value )
{
    Modify( m_ivProgram, value, eInfo );
}

void NxModel::SetDesign( const QString& value )
{
    Modify( m_design, value, eInfo );
}

void NxModel::SetDesign( size_t value )
{
    SetDesign( QString::number( value ).rightJustified( 4, '0' ) );
}

void NxModel::SetCvWidth( double value )
{
    Modify( m_cvWidth, value, eInfo );
}

void NxModel::SetStopper( int value )
{
    Modify( m_stopper, value, eInfo );
}

void NxModel::SetPositions( const std::map<int, NxPosition>& value )
{
    m_positions = value;
    SetModified( ePosition );
}

void NxModel::SetPosition( int index, const NxPosition& value )
{
    m_positions[ index ] = value;
    SetModified( ePosition );
}

void NxModel::RemovePosition( int index )
{
    m_positions.erase( index );
    SetModified( ePosition );
}

void NxModel::SetComments( const std::map<QString, QString>& comments )
{
    m_comments = comments;
    SetModified( eComment );
}

void NxModel::SetComment( const QString& key, const QString& value )
{
    m_comments[ key.trimmed().toUpper() ] = value;
    SetModified( eComment );
}

void NxModel::AddComments( const QStringList& keys )
{
    for ( auto& key : keys )
    {
        auto it = m_comments.find( key.trimmed().toUpper() );
        if ( it == m_comments.end() )
        {
            m_comments[ key.trimmed().toUpper() ] = "";
            SetModified( eComment );
        }
    }
}

void NxModel::RemoveComments( const QStringList& keys )
{
    for ( auto& key : keys )
    {
        auto it = m_comments.find( key );
        if ( it != m_comments.end() )
        {
            m_comments.erase( key );
            SetModified( eComment );
        }

        it = m_comments.find( key.trimmed().toUpper() );
        if ( it != m_comments.end() )
        {
            m_comments.erase( key.trimmed().toUpper() );
            SetModified( eComment );
        }
    }
}

void NxModel::SetValue( const QString& key, const QString& value )
{
    m_comments[ key.trimmed().toUpper() ] = value;
    SetModified( eComment );
}

QStringList NxModel::paramNames()
{
    QStringList names = { "NAME","CODE","NO","FIX1","FIX2","FIX3","FIX4","FIX5" };
    // for ( auto& it : items )
    // {
    //     for ( auto& [key, value] : it->m_comments )
    //     {
    //         if ( names.contains( key ) == false )
    //         {
    //             names.push_back( key );
    //         }
    //     }
    // }
    return names;
}

NxModelManager::NxModelManager() :QObject( nullptr )
{
    m_settings.Load();
}

NxModelPtr NxModelManager::Create( const QString& code, const QString& name )
{
    auto pModel = std::make_shared<NxModel>();
    pModel->SetCode( code );
    pModel->SetName( name );
    return pModel;
}

QStringList NxModelManager::Names()
{
    QStringList items;
    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return items;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
        return items;
    NxQuery query( db );

    QString cmd = QString( "SELECT Name FROM Models" );
    if ( !query.exec( cmd ) )
    {
        db.close();
        return items;
    }

    while ( query.next() )
    {
        items.push_back( query.value( 0 ).toString() );
    }
    db.close();
    return items;
}

QStringList NxModelManager::ParamNames()
{
    QStringList items;
    return items;
}

NxModelPtrMap NxModelManager::GetModels()
{
    NxModelPtrMap map;
    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return map;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
        return map;
    NxQuery query( db );

    QString cmd = QString( "SELECT Code,Name,Design,kNo,IV,CvWidth,Stopper,IsPrintLo FROM Models" );
    if ( !query.exec( cmd ) )
    {
        db.close();
        qDebug() << query.lastError().text();
        return map;
    }

    while ( query.next() )
    {
        NxModelPtr pModel = Create();
        pModel->SetCode( query.value( 0 ).toString() );
        pModel->SetName( query.value( 1 ).toString() );
        pModel->SetDesign( query.value( 2 ).toString() );
        pModel->SetkNo( query.value( 3 ).toString() );
        pModel->SetIVProgram( query.value( 4 ).toString() );
        pModel->SetCvWidth( query.value( 5 ).toDouble() );
        pModel->SetStopper( query.value( 6 ).toInt() );
        pModel->SetPrintLo( query.value( 7 ).toInt() > 0 );
        pModel->ClearModified();
        map[ pModel->Code() ] = pModel;
    }

    return map;
}

NxModelPtr NxModelManager::GetModel( const QString& code )
{
    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return nullptr;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
        return nullptr;
    NxQuery query( db );

    QString cmd = QString( "SELECT Code,Name,Design,kNo,IV,CvWidth,Stopper,IsPrintLo "
                           "FROM Models "
                           "WHERE Code='%1' OR Name='%1'" )
        .arg( code );

    if ( !query.exec( cmd ) )
    {
        qDebug() << query.lastError().text();
        db.close();
        return nullptr;
    }

    NxModelPtr pModel;
    while ( query.next() )
    {
        if ( !pModel )
        {
            pModel = Create();
            pModel->SetCode( query.value( 0 ).toString() );
            pModel->SetName( query.value( 1 ).toString() );
            pModel->SetDesign( query.value( 2 ).toString() );
            pModel->SetkNo( query.value( 3 ).toString() );
            pModel->SetIVProgram( query.value( 4 ).toString() );
            pModel->SetCvWidth( query.value( 5 ).toDouble() );
            pModel->SetStopper( query.value( 6 ).toInt() );
            pModel->SetPrintLo( query.value( 7 ).toInt() > 0 );
        }
    }

    if ( !pModel )
    {
        db.close();
        return pModel;
    }

    cmd = QString( "SELECT Number,X,Y,Angle FROM ModelPositions WHERE Code='%1'" ).arg( pModel->Code() );
    if ( query.exec( cmd ) )
    {
        while ( query.next() )
        {
            NxPosition pos;
            pos.index = query.value( 0 ).toInt();
            pos.x = query.value( 1 ).toDouble();
            pos.y = query.value( 2 ).toDouble();
            pos.angle = query.value( 3 ).toInt();
            pModel->SetPosition( pos.index, pos );
        }
    }

    cmd = QString( "SELECT Key,Value FROM ModelParams WHERE Code='%1'" ).arg( pModel->Code() );
    if ( query.exec( cmd ) )
    {
        while ( query.next() )
        {
            QString key = query.value( 0 ).toString();
            QString value = query.value( 1 ).toString();
            pModel->SetValue( key, value );
        }
    }

    pModel->ClearModified();
    return pModel;
}

ReturnCode NxModelManager::Save( NxModelPtr pModel )
{
    if ( !pModel )
        return RtDataNull;

    if ( !pModel->IsMofified() )
        return RtDataNoChanges;

    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return RtDBFileNotFound;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
    {
        qDebug() << "Save Model: " << db.lastError().text();
        return RtDBOpenFailed;
    }

    NxQuery query( db );

    if ( pModel->IsMofified( NxModel::eNew ) )
    {
        QString cmd = QString( "INSERT INTO Models "
                               "(Code,Name,Design,kNo,IV,CvWidth,Stopper,IsPrintLo) "
                               "VALUES('%1','%2','%3','%4','%5','%6','%7','%8'); " )
            .arg( pModel->Code() )
            .arg( pModel->Name() )
            .arg( pModel->Design() )
            .arg( pModel->kNo() )
            .arg( pModel->IVProgram() )
            .arg( pModel->CvWidth() )
            .arg( pModel->Stopper() )
            .arg( pModel->IsPrintLo() );
        if ( !query.exec( cmd ) )
        {
            qDebug() << "Save Model: " << query.lastError().text();
            db.close();
            return RtDBQueryFailed;
        }
    }
    else if ( pModel->IsMofified( NxModel::eInfo ) )
    {
        QString cmd = QString( "UPDATE Models "
                               "SET Name = '%1',Design = '%2',kNo = '%3',IV = '%4',CvWidth = '%5',Stopper = '%6',IsPrintLo = '%7' "
                               "WHERE Code = '%8'" )
            .arg( pModel->Name() )
            .arg( pModel->Design() )
            .arg( pModel->kNo() )
            .arg( pModel->IVProgram() )
            .arg( pModel->CvWidth() )
            .arg( pModel->Stopper() )
            .arg( pModel->IsPrintLo() )
            .arg( pModel->Code() );
        if ( !query.exec( cmd ) )
        {
            qDebug() << "Save Model: " << query.lastError().text();
            db.close();
            return RtDBQueryFailed;
        }
    }

    if ( pModel->IsMofified( NxModel::ePosition ) )
    {
        QString cmd = QString( "DELETE FROM ModelPositions WHERE Code = '%1'" ).arg( pModel->Code() );
        query.exec( cmd );

        auto& positions = pModel->Positions();
        for ( auto& [index, position] : positions )
        {
            cmd = QString( "INSERT INTO ModelPositions "
                           "(Code,Number,X,Y,Angle) "
                           "VALUES('%1','%2','%3','%4','%5')" )
                .arg( pModel->Code() )
                .arg( index )
                .arg( position.x )
                .arg( position.y )
                .arg( position.angle );
            if ( !query.exec( cmd ) )
            {
                qDebug() << "Save Model: " << query.lastError().text();
                db.close();
                return RtDBQueryFailed;
            }
        }
    }

    if ( pModel->IsMofified( NxModel::eComment ) )
    {
        QString cmd = QString( "DELETE FROM ModelParams WHERE Code = '%1'" ).arg( pModel->Code() );
        query.exec( cmd );

        auto& comments = pModel->Comments();
        for ( auto& [key, value] : comments )
        {
            cmd = QString( "INSERT INTO ModelParams "
                           "(Code,Key,Value) "
                           "VALUES('%1','%2','%3')" )
                .arg( pModel->Code() )
                .arg( key )
                .arg( value );
            if ( !query.exec( cmd ) )
            {
                qDebug() << "Save Model: " << query.lastError().text();
                db.close();
                return RtDBQueryFailed;
            }
        }
    }
    pModel->ClearModified();
    db.close();
    return RtNormal;
}

ReturnCode NxModelManager::Delete( const QString& code )
{
    QString dbFilePath = m_settings.String( DatabaseFilePath );
    if ( !NxDatabase::CheckDatabaseFileExisting( dbFilePath ) )
        return RtDBFileNotFound;

    NxDatabase db = NxDatabase::database( "SQLITE" );
    db.setDatabaseName( dbFilePath );
    if ( !db.open() )
        return RtDBOpenFailed;

    NxQuery query( db );
    QString cmd = QString( "DELETE FROM Models WHERE Code = '%1'" ).arg( code );
    if ( !query.exec( cmd ) )
    {
        qDebug() << query.lastError().text();
    }

    cmd = QString( "DELETE FROM ModelPositions WHERE Code = '%1'" ).arg( code );
    if ( !query.exec( cmd ) )
    {
        qDebug() << query.lastError().text();
    }

    cmd = QString( "DELETE FROM ModelParams WHERE Code = '%1'" ).arg( code );
    if ( query.exec( cmd ) )
    {
        qDebug() << query.lastError().text();
    }

    db.close();
    return RtNormal;
}

void NxModelManager::AddComments( const QStringList& keys )
{

}

void NxModelManager::RemoveComments( const QStringList& keys )
{

}

ReturnCode NxModelManager::DeleteAll()
{
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

    cmd = QString( "DELETE FROM Models" );
    if ( !query.exec( cmd ) )
    {
        db.close();
        return RtDBQueryFailed;
    }

    cmd = QString( "DELETE FROM ModelParams" );
    if ( !query.exec( cmd ) )
    {
        db.close();
        return RtDBQueryFailed;
    }

    cmd = QString( "DELETE FROM ModelPositions" );
    if ( !query.exec( cmd ) )
    {
        db.close();
        return RtDBQueryFailed;
    }

    db.close();
    return RtNormal;
}

void NxModelManager::ReloadSetting()
{
    m_settings.Load();
}

NxModelManager* ModelManager()
{
    static NxModelManager instance;
    return &instance;
}
