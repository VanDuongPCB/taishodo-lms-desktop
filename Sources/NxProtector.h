#pragma once
#include "QObject"
#include "QString"
#include "NxProfile.h"

class NxProtector : public QObject
{
    Q_OBJECT
public:
    explicit NxProtector();
    ~NxProtector();
    bool Login( QString name, QString pass );
    void Logout();
    NxProfilePtr Profile();
private:
    HxRegistrySetting m_settings;
    NxProfilePtr m_pProfile;
    bool eventFilter( QObject* watched, QEvent* event );
};


NxProtector* Protector();
