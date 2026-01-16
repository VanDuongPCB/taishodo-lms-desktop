
#include "NxException.h"
#include "QDateTime"
#include "QJsonObject"
#include "QJsonDocument"

NxException::NxException() : QException()
{
    m_time = QDateTime::currentDateTime().toString( "yyyy/MM/dd HH:mm:ss" );
}

NxException::NxException( const QString& what ) : NxException()
{
    this->m_message = what;
}

NxException::NxException( const QString& where, const QString& what ) : NxException( what )
{
    this->m_where = where;
}

QString NxException::Time() const
{
    return m_time;
}

QString NxException::Where() const
{
    return m_where;
}

QString NxException::Message() const
{
    return m_message;
}

void NxException::SetWhere( const QString& where )
{
    m_where = where;
}

void NxException::SetMessage( const QString& message )
{
    m_message = message;
}

QJsonObject NxException::toJsonObject()
{
    QJsonObject obj;
    obj.insert( "time", m_time );
    obj.insert( "where", m_where );
    obj.insert( "message", m_message );
    return obj;
}