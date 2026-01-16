#pragma once
#include "QString"
#include "QDir"
#include "QFile"
#include "QFileInfo"
#include "QSettings"

QStringList GetFilesAndFolders( const QString& path, QDir::Filters filters = QDir::Dirs | QDir::Files );