#pragma once
#include "QSQlDatabase"
#include "QSqlQuery"
#include "QSqlError"

using NxQuery = QSqlQuery;

class NxDatabase : public QSqlDatabase
{
public:
    NxDatabase();
    NxDatabase( const QSqlDatabase& other );
    NxDatabase( const QString& fileDB );
    static bool CheckDatabaseFileExisting( const QString& file );
};
