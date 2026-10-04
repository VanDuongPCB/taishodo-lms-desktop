#include "NxSystemError.h"



NxSystemError::NxSystemError()
{

}


void NxSystemError::ErrorReport( NxException ex )
{
    emit Reported( ex );
}

NxSystemError* NxSystemError::Instance()
{
    static NxSystemError _ins;
    return &_ins;
}
