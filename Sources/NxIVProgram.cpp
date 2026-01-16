#include "NxIVProgram.h"
#include "NxSettings.h"
#include "QDir"
#include "QFileInfo"
#include "QFileInfoList"
#include "QCoreApplication"

#include "NxFileManager.h"

NxIVProgramArray NxIVProgramManager::Get()
{
    NxIVProgramArray items;

    HxRegistrySetting setting;
    QString ivDir = setting.String( IVProgramDir );

    QStringList files = QDir( ivDir ).entryList( { "*.iva" }, QDir::Files | QDir::NoSymLinks );
    for ( int i = 0; i < files.size(); i++ )
    {
        QFileInfo fi( ivDir + "/" + files[ i ] );
        NxIVProgram program;
        program.Name = fi.baseName();
        program.FilePath = fi.absoluteFilePath();
        program.FileSize = fi.size();
        program.LastTimeModified = fi.lastModified();
        items.push_back( program );
    }

    return items;
}


NxIVProgramManager* IVProgram()
{
    static NxIVProgramManager s_instance;
    return &s_instance;
}