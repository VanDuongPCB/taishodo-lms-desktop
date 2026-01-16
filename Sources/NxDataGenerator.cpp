#include "NxDataGenerator.h"

QString GenMarkData( const QString& format, NxLOTPtr pLOT, NxModelPtr pModel, int shift )
{
    if ( format.isEmpty() || !pLOT || !pModel )
        return "";

    QStringList formatItems = format.split( ',' );
    QStringList itemDatas;
    for ( auto& formatItem : formatItems )
    {
        QStringList itemParams = formatItem.split( '.' );
        if ( itemParams.size() != 2 )
        {
            itemDatas.push_back( formatItem );
        }
        else
        {
            if ( itemParams[ 0 ].trimmed().length() < 1 || itemParams[ 1 ].trimmed().length() < 1 )
            {
                itemDatas.push_back( formatItem );
            }
            QString objectName = itemParams[ 0 ].trimmed().toLower();
            if ( objectName == "time" )
            {
                itemDatas.push_back( QDateTime::currentDateTime().toString( itemParams[ 1 ] ) );
            }
            else
            {
                QString paramName = itemParams[ 1 ].toUpper().trimmed();
                if ( objectName == "model" )
                {
                    QString value = pModel->Value( paramName );
                    itemDatas.push_back( value );
                }
                else if ( objectName == "lot" )
                {
                    QString value = pLOT->Value( paramName );
                    if ( paramName == "COUNTER" )
                        value = pLOT->Counter( shift );
                    else if ( paramName == "MAC" )
                        value = pLOT->MAC( shift );
                    itemDatas.push_back( value );
                }
            }
        }
    }
    QString data = itemDatas.join( ',' );
    return data.replace( ",", "" );
}

std::map<int, QString> GenMarkData( NxDesignPtr pDesign, NxLOTPtr pLOT, NxModelPtr pModel, int shift )
{
    std::map<int, QString> items;
    if ( !pLOT || !pDesign || !pModel )
        return items;

    for ( auto& [index, block] : pDesign->Blocks() )
    {
        if ( index < 1 )
            continue;
        QString format = block.data.trimmed();
        if ( format.length() < 1 ) continue;
        QString data = GenMarkData( format, pLOT, pModel, shift );
        items[ index ] = data.replace( ",", "" );
    }
    return items;
}