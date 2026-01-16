#pragma once
#include "QEvent"
#include "QVariant"

class NxEvent : public QEvent
{
public:
    enum Type
    {
        eCustomEvent,

        eDatabaseError,

        eLoginEvent,
        eLogoutEvent,

        eDesignAdded,
        eDesignDeleted,
        eDesignChanged,

        eModelAdded,
        eModelDeleted,
        eModelChanged,

        eLOTAdded,
        eLOTDeleted,
        eLOTChanged,

        eSettingChanged,

        eMarkerSetupChanged,
        eMarkerGoFree,
        eMarkerGoError,
        eMarkerGoTransfer,
        eMarkerGoMark,
        eMarkerGoTest,
        eMarkerMarked,
        eMarkerGoFinish,
        eMarkerStopped,
    };
    NxEvent( Type type );
    NxEvent( Type type, QVariant data );
    ~NxEvent();
    Type GetType() const;
    QVariant Data() const;
private:
    Type m_type = eCustomEvent;
    QVariant m_data;

public:
    static bool IsCustomEvent( QEvent* event, NxEvent*& pEvent, NxEvent::Type& type );
};