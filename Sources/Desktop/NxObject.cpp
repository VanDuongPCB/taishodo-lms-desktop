#include "NxObject.h"

NxObject::NxObject()
{
}

void NxObject::SetModified( uint64_t flags )
{
    m_flags |= flags;
}

bool NxObject::IsMofified( uint64_t flags ) const
{
    return ( m_flags & flags ) > 0;
}

uint64_t NxObject::ModifyFlags() const
{
    return m_flags;
}

void NxObject::ClearModified()
{
    m_flags = 0;
}
