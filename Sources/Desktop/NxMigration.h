#pragma once
#include "QString"

void CheckAndCopyDB();
void MigrateLOTs( const QString& dir );
void MigrateModels( const QString& dir );
void MigrateDesigns( const QString& dir );
void MigratePrintLogs( const QString& dir );
void MigrateRePrintLogs( const QString& dir );
void MigrateBarcodes( const QString& dir );
void MigrateStoppers( const QString& file );