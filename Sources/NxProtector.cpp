#include "NxProtector.h"

#include "QApplication"

#include "NxSettings.h"
#include "NxFileManager.h"
#include "NxMessage.h"
#include "NxEvent.h"

NxProtector::NxProtector() : QObject( nullptr )
{
    m_settings.Load();
    qApp->installEventFilter( this );
}

NxProtector::~NxProtector()
{}

bool NxProtector::Login( QString name, QString pass )
{
    if ( pass.isEmpty() || name.isEmpty() )
    {
        HxMsgError( tr( "Mật khẩu hoặc tên không được để trống!" ) );
        return false;
    }
    QString passwordKey = "Protect/" + name + "Password";
    QString userName = name.trimmed().toLower();
    QString setupPassword = m_settings.String( passwordKey );

    if ( pass == setupPassword )
    {
        m_pProfile = NxProfile::Create();
        m_pProfile->SetID( name.trimmed().toLower() );
        m_pProfile->SetName( name );
        m_pProfile->SetPassword( pass );

        if ( name == "Admin" )
        {
            m_pProfile->SetPermission( "ADMIN", true);
        }
        else if ( name == "Super" )
        {
            m_pProfile->SetPermission( "SUPER", true );
        }
        else if ( name == "Leader" )
        {
            m_pProfile->SetPermission( "LEADER", true );
        }

        qApp->postEvent( qApp, new NxEvent( NxEvent::eLoginEvent ) );
        return true;
    }
    else
    {
        HxMsgError( tr( "Mật khẩu không đúng!" ) );
        return false;
    }
    return false;
}

void NxProtector::Logout()
{
    m_pProfile.reset();
    qApp->postEvent( qApp, new NxEvent( NxEvent::eLogoutEvent ) );
}

NxProfilePtr NxProtector::Profile()
{
    return m_pProfile;
}

bool NxProtector::eventFilter( QObject* watched, QEvent* event )
{
    NxEvent* NxEvent( nullptr );
    NxEvent::Type type;
    if ( !NxEvent::IsCustomEvent( event, NxEvent, type ) )
        return QObject::eventFilter( watched, event );

    switch ( type )
    {
    case NxEvent::eSettingChanged:
        m_settings.Load();
        break;
    default:
        break;
    }
    return QObject::eventFilter( watched, event );
}

NxProtector* Protector()
{
    static NxProtector instance;
    return &instance;
}
