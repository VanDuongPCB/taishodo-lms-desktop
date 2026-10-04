#include "NxSystemReport.h"

namespace
{
    NxSystemReport s_instance;
}

NxSystemReport::NxSystemReport()
{

}

void NxSystemReport::Report( NxException ex )
{
    emit reported( ex );
}

NxSystemReport* GetSystemReport()
{
    return &s_instance;
}