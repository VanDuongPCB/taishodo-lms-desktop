#include "NxMessage.h"
#include "QMessageBox"
#include "QAbstractButton"
#include "NxTheme.h"

namespace
{
    std::map<HxMsgButton, QString> s_buttonNames =
    {
        {HxMsgButton::Ok,"Ok"},
        {HxMsgButton::Save,"Save"},
        {HxMsgButton::SaveAll,"SaveAll"},
        {HxMsgButton::Open,"Open"},
        {HxMsgButton::Yes,"Yes"},
        {HxMsgButton::YesToAll,"YesToAll"},
        {HxMsgButton::No,"No"},
        {HxMsgButton::NoToAll,"NoToAll"},
        {HxMsgButton::Abort,"Abort"},
        {HxMsgButton::Retry,"Retry"},
        {HxMsgButton::Ignore,"Ignore"},
        {HxMsgButton::Close,"Close"},
        {HxMsgButton::Cancel,"Cancel"},
        {HxMsgButton::Discard,"Discard"},
        {HxMsgButton::Help,"Help"},
        {HxMsgButton::Apply,"Apply"},
        {HxMsgButton::Reset,"Reset"},
        {HxMsgButton::RestoreDefaults,"RestoreDefaults"},

        {HxMsgButton::FirstButton,"FirstButton"},
        {HxMsgButton::LastButton,"LastButton"},

        {HxMsgButton::YesAll,"YesAll"},
        {HxMsgButton::NoAll,"NoAll"},

        {HxMsgButton::Default,"Default"},
        {HxMsgButton::Escape,"Escape"},
        {HxMsgButton::FlagMask,"FlagMask"},
        {HxMsgButton::ButtonMask,"ButtonMask"}
    };

    void ApplyTheme( QMessageBox& msgBox, const QString& objectName, bool bIsHighlight = false )
    {
        msgBox.setObjectName( objectName + ( bIsHighlight ? "High" : "" ) );
        for ( auto& id : s_buttonNames )
        {
            QAbstractButton* pButton = msgBox.button( id.first );
            if ( pButton )
                pButton->setObjectName( "HxMsgButton" + id.second );
        }
        msgBox.setStyleSheet( Theme()->GetStyleSheets( { "messagebox" } ) );
    }
}

int HxMsgShow( const QString& content )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( QObject::tr( "Thông báo" ) );
    msgBox.setText( content );
    msgBox.setIcon( HxMsgIcon::NoIcon );
    msgBox.setStandardButtons( QMessageBox::Ok );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgNormal" );
    return msgBox.exec();
}

int HxMsgShow( const QString& content, const QString& title )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( title );
    msgBox.setText( content );
    msgBox.setIcon( HxMsgIcon::NoIcon );
    msgBox.setStandardButtons( QMessageBox::Ok );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgNormal" );
    return msgBox.exec();
}

int HxMsgShow( const QString& content, const QString& title, HxMsgIcon icon )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( title );
    msgBox.setText( content );
    msgBox.setIcon( icon );
    msgBox.setStandardButtons( QMessageBox::Ok );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgNormal" );
    return msgBox.exec();
}

int HxMsgInfo( const QString& content )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( QObject::tr( "Thông tin" ) );
    msgBox.setText( content );
    msgBox.setIcon( HxMsgIcon::Information );
    msgBox.setStandardButtons( QMessageBox::Ok );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgInfo" );
    return msgBox.exec();
}
int HxMsgInfo( const QString& content, const QString& title )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( title );
    msgBox.setText( content );
    msgBox.setIcon( HxMsgIcon::Information );
    msgBox.setStandardButtons( QMessageBox::Ok );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgInfo" );
    return msgBox.exec();
}

int HxMsgQuestion( const QString& content )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( QObject::tr( "Xác nhận" ) );
    msgBox.setText( content );
    msgBox.setIcon( HxMsgIcon::Question );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    ApplyTheme( msgBox, "HxMsgQuestion" );
    return msgBox.exec();
}
int HxMsgQuestion( const QString& content, const QString& title )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( title );
    msgBox.setText( content );
    msgBox.setIcon( HxMsgIcon::Question );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    ApplyTheme( msgBox, "HxMsgQuestion" );
    return msgBox.exec();
}

int HxMsgQuestion( const QString& content, const QString& title, HxMsgButtons buttons )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( title );
    msgBox.setText( content );
    msgBox.setStandardButtons( buttons );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgQuestion" );
    return msgBox.exec();
}

int HxMsgWarning( const QString& content, bool bIsHighlight )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( QObject::tr( "Cảnh báo" ) );
    msgBox.setText( content );
    msgBox.setIcon( HxMsgIcon::Warning );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgWarning", bIsHighlight );
    return msgBox.exec();
}

int HxMsgWarning( const QString& content, const QString& title, bool bIsHighlight )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( title );
    msgBox.setText( content );
    msgBox.setIcon( HxMsgIcon::Warning );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgWarning", bIsHighlight );
    return msgBox.exec();
}

int HxMsgError( const QString& content, bool bIsHighlight )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( QObject::tr( "Lỗi" ) );
    msgBox.setText( content );
    msgBox.setIcon( HxMsgIcon::Critical );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgCritical", bIsHighlight );
    return msgBox.exec();
}

int HxMsgError( const QString& content, const QString& title, bool bIsHighlight )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( title );
    msgBox.setText( content );
    msgBox.setIcon( HxMsgIcon::Critical );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgCritical", bIsHighlight );
    return msgBox.exec();
}

int HxMsgError( const QString& content, const QString& details, const QString& title, bool bIsHighlight )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( title );
    msgBox.setText( content );
    msgBox.setDetailedText( details );
    msgBox.setIcon( HxMsgIcon::Critical );
    msgBox.setDefaultButton( QMessageBox::Ok );
    ApplyTheme( msgBox, "HxMsgCritical", bIsHighlight );
    return msgBox.exec();
}


