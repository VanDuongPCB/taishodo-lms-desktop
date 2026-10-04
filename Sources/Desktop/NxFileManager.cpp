#include "NxFileManager.h"
#include "QCoreApplication"
#include "QDir"
#include "QDirIterator"
#include "QSettings"

QStringList GetFilesAndFolders( const QString& path, QDir::Filters filters )
{
    QStringList items;
    QDirIterator it( path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories );

    while ( it.hasNext() )
    {
        QString entryPath = it.next();
        QFileInfo info( entryPath );

        if ( filters & QDir::Dirs && info.isDir() )
            items.push_back( entryPath );
        else if ( filters & QDir::Files && info.isFile() )
            items.push_back( entryPath );
    }
    return items;
}