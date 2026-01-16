#pragma once
#include "QException"
#include "QJsonObject"


class NxException : public QException
{
private:
public:
    NxException();
    NxException( const QString& what );
    NxException( const QString& where, const QString& what );

    QString Time() const;
    QString Where() const;
    QString Message() const;

    void SetWhere( const QString& where );
    void SetMessage( const QString& message );
    QJsonObject toJsonObject();

private:
    QString m_time = "0000/00/00 00:00:00";
    QString m_where = "";
    QString m_message = "";
};