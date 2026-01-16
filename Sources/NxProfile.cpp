#include "NxProfile.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "QStringBuilder"

#include "NxDatabase.h"
#include "NxFileManager.h"
#include "NxSettings.h"

NxProfile::NxProfile()
{

}

NxProfile::~NxProfile()
{

}

QString NxProfile::ID() const
{
    return m_id.trimmed().toLower();
}
QString NxProfile::Name() const
{
    return m_name;
}

QString NxProfile::Password() const
{
    return m_password;
}

QString NxProfile::Permissions() const
{
    QStringList items;
    for ( auto& permission : m_permissions )
    {
        items.push_back( permission.trimmed().toUpper() );
    }
    return items.join( "," );
}

bool NxProfile::Permission( const QString& permission ) const
{
    QString permisionName = permission.trimmed().toUpper();
    auto it = m_permissions.find( permisionName );
    return it != m_permissions.end();
}

void NxProfile::SetID( const QString& ID )
{
    Modify( m_id, ID, eNew );
}
void NxProfile::SetName( const QString& name )
{
    Modify( m_name, name, eName );
}

void NxProfile::SetPassword( const QString& password )
{
    Modify( m_password, password, ePassword );
}

void NxProfile::SetPermission( const QString& permission, bool isOn )
{
    QString permisionName = permission.trimmed().toUpper();
    if ( permisionName.isEmpty() )
        return;
    if ( isOn )
    {
        auto it = m_permissions.find( permisionName );
        if ( it == m_permissions.end() )
        {
            m_permissions.insert( permisionName );
            SetModified( ePermission );
        }
    }
    else
    {
        auto it = m_permissions.find( permisionName );
        if ( it != m_permissions.end() )
        {
            m_permissions.erase( permisionName );
            SetModified( ePermission );
        }
    }
}

NxProfilePtr NxProfile::Create()
{
    return std::make_shared<NxProfile>();
}