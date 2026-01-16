#pragma once
#include "NxEvent.h"

NxEvent::NxEvent( NxEvent::Type type ) :
    QEvent( QEvent::Type::User ),
    m_type( type )
{
}

NxEvent::NxEvent( Type type, QVariant data ) :
    QEvent( QEvent::Type::User ),
    m_type( type ),
    m_data( data )
{
}

NxEvent::~NxEvent()
{
}

NxEvent::Type NxEvent::GetType() const
{
    return m_type;
}

QVariant NxEvent::Data() const
{
    return m_data;
}

bool NxEvent::IsCustomEvent( QEvent* event, NxEvent*& pEvent, NxEvent::Type& type )
{
    if ( !event || event->type() != NxEvent::User )
        return false;

    pEvent = static_cast< NxEvent* >( event );
    if ( !pEvent )
        return false;

    type = pEvent->GetType();
    return true;
}