#include "NxMainWindow.h"
#include "NxDebugFlags.h"
#include "ui_NxMainWindow.h"
#include "XTabBar.h"

#include "QSignalBlocker"
#include "QCloseEvent"
#include "QCoreApplication"

#include "NxSettingsWindow.h"
#include "NxMarkWindow.h"
#include "NxControlWindow.h"
#include "NxModelWindow.h"
#include "NxLOTWindow.h"
#include "NxDesignWindow.h"
#include "NxTransferWindow.h"
#include "NxLoginDialog.h"
#include "NxIVProgramWindow.h"
#include "NxLogWindow.h"

#include "NxProtector.h"
#include "NxMessage.h"
#include "NxEvent.h"
#include "NxSystemError.h"
#include "NxLicense.h"
#include "NxLOT.h"
#include "NxMarker.h"
#include "NxLogger.h"

#include "NxTheme.h"

namespace TabIndex
{
    constexpr int Mark = 0;
    constexpr int Transfer = 1;
    constexpr int Plans = 2;
    constexpr int Model = 3;
    constexpr int Design = 4;
    constexpr int IVProgram = 5;
    constexpr int Settings = 6;
    constexpr int Control = 7;
    constexpr int Data = 8;
}

NxMainWindow::NxMainWindow( QWidget* parent ) : XMainWindow( parent ), ui( new Ui::MainWindow )
{
    ui->setupUi( this );

    // Tab chính: icon nằm trên chữ (XTabWidget, xem XTabBar::setToolButtonStyle).
    ui->tabWidget->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    ui->tabWidget->setIconTextSpacing( 4 );
    ui->tabWidget->setRotateVerticalText( false );  // tab dọc (West/East): chữ vẫn nằm ngang dưới icon, không xoay 90°
    ui->tabWidget->setUsesScrollButtons( false );   // cửa sổ hẹp: tab tự co và rút gọn chữ, không hiện nút cuộn

    // Nút Đăng nhập/Đăng xuất thay cho tab đầu tiên: nằm trong hàng tab, góc trái,
    // cao bằng tab; dãy tab tự dịch sang phải đúng bằng bề rộng nút.
    m_pLoginButton = new XToolButton( ui->tabWidget );
    m_pLoginButton->setObjectName( "btnLogin" );
    m_pLoginButton->setIcon( QIcon( ":/protect-01.png" ) );
    m_pLoginButton->setIconSize( ui->tabWidget->iconSize() );
    m_pLoginButton->setText( tr( "Đăng nhập" ) );
    m_pLoginButton->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    m_pLoginButton->setAutoRaise( true );
    m_pLoginButton->setCursor( Qt::PointingHandCursor );
    ui->tabWidget->setTabBarCornerWidget( m_pLoginButton, Qt::TopLeftCorner );   // cao bằng tab, thẳng hàng với tab
    connect( m_pLoginButton, &QToolButton::clicked, this, &NxMainWindow::OnLoginOrLogout );

    m_pMarkWindow = new NxMarkWindow();
    m_pTransferWindow = new NxTransferWindow();
    m_pLOTWindow = new NxLOTWindow();
    m_pModelWindow = new NxModelWindow();
    m_pDesignWindow = new NxDesignWindow();
    m_pIVProgramWindow = new NxIVProgramWindow();
    m_pSettingsWindow = new NxSettingsWindow();
    m_pControlWindow = new NxControlWindow();
    m_pLogWindow = new NxLogWindow();

    ui->tabMark->layout()->addWidget( m_pMarkWindow );
    ui->tabTransfer->layout()->addWidget( m_pTransferWindow );
    ui->tabPlans->layout()->addWidget( m_pLOTWindow );
    ui->tabModel->layout()->addWidget( m_pModelWindow );
    ui->tabDesign->layout()->addWidget( m_pDesignWindow );
    ui->tabIV->layout()->addWidget( m_pIVProgramWindow );
    ui->tabSetting->layout()->addWidget( m_pSettingsWindow );
    ui->tabControl->layout()->addWidget( m_pControlWindow );
    ui->tabData->layout()->addWidget( m_pLogWindow );

    qApp->installEventFilter( this );

    m_currentTabIndex = ui->tabWidget->currentIndex();

    connect( NxSystemError::Instance(), &NxSystemError::Reported, this, &NxMainWindow::ErrorReported );
    connect( ui->tabWidget, &QTabWidget::currentChanged, this, &NxMainWindow::OnTabChanged );

    Marker()->moveToThread( QCoreApplication::instance()->thread() );
    Marker()->Init();

    if ( !IsLoginSkipped() )
        OnLockUI();

    Theme()->Refresh();   // nạp theme đã lưu trong registry (mặc định "Light", từ resource :/themes)
    setWindowState( Qt::WindowMaximized );
}

NxMainWindow::~NxMainWindow()
{
    Marker()->DeInit();
    delete ui;
}

void NxMainWindow::resizeEvent( QResizeEvent* event )
{
    if ( !m_pLblVersion )
    {
        m_pLblVersion = new QLabel( this );
        m_pLblVersion->setGeometry( 0, 0, 100, 35 );
        m_pLblVersion->setText( tr("Phiên bản\n") + Licensing()->GetVersion());
        m_pLblVersion->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
        m_pLblVersion->setStyleSheet( "color:#808; font-weight:bold;" );
    }
    QRect mainGeo = geometry();
    m_pLblVersion->setGeometry( mainGeo.width() - m_pLblVersion->width()-5, 5, 100, 35 );
}

void NxMainWindow::closeEvent( QCloseEvent* event)
{
    if ( HxMsgQuestion( tr( "Xác nhận thoát chương trình" ), tr( "Thoát chương trình?" ) ) != HxMsgButton::Yes )
    {
        event->ignore();
        return;
    }
}

bool NxMainWindow::eventFilter( QObject* watched, QEvent* event )
{
    NxEvent* NxEvent( nullptr );
    NxEvent::Type type;
    if ( !NxEvent::IsCustomEvent( event, NxEvent, type ) )
        return XMainWindow::eventFilter( watched, event );

    switch ( type )
    {
    case NxEvent::eLoginEvent:
    case NxEvent::eLogoutEvent:
    {
        NxProfilePtr pProfile = Protector()->Profile();
        if ( !pProfile )
            OnLockUI();
        else if ( pProfile->Permission( "ADMIN" ) )
            OnUnLockUIForAdmin();
        else if ( pProfile->Permission( "SUPER" ) )
            OnUnLockUIForSuper();
        else if ( pProfile->Permission( "LEADER" ) )
            OnUnLockUIForLeader();
    }
        break;
    default:
        break;
    }

    return XMainWindow::eventFilter( watched, event );
}

void NxMainWindow::OnLoginOrLogout()
{
    if ( Protector()->Profile() != nullptr )
    {
        int res = HxMsgQuestion( "Bạn có chắc chắn muốn đăng xuất không ?", "Khoan đã" );
        if ( res == QMessageBox::StandardButton::Yes )
            Protector()->Logout();
    }
    else
    {
        NxLoginDialog( this ).exec();
    }
}

void NxMainWindow::OnLockUI()
{
    OnUnLockUIForAdmin();
    m_pLoginButton->setText( tr( "Đăng nhập" ) );
    SetTabVisible( ui->tabPlans, false );
    SetTabVisible( ui->tabModel, false );
    SetTabVisible( ui->tabDesign, false );
    SetTabVisible( ui->tabSetting, false );
    SetTabVisible( ui->tabControl, false );
}

void NxMainWindow::OnUnLockUIForLeader()
{
    OnUnLockUIForAdmin();
    SetTabVisible( ui->tabModel, false );
    SetTabVisible( ui->tabDesign, false );
    SetTabVisible( ui->tabSetting, false );
    SetTabVisible( ui->tabControl, false );
}

void NxMainWindow::OnUnLockUIForSuper()
{
    OnUnLockUIForAdmin();
    SetTabVisible( ui->tabSetting, false );
    SetTabVisible( ui->tabControl, false );
}

void NxMainWindow::OnUnLockUIForAdmin()
{
    m_pLoginButton->setText( tr( "Đăng xuất" ) );
    for ( int i = 0; i < ui->tabWidget->count(); i++ )
        ui->tabWidget->setTabVisible( i, true );
}

void NxMainWindow::SetTabVisible( QWidget* pPage, bool bVisible )
{
    int index = ui->tabWidget->indexOf( pPage );
    if ( index >= 0 )
        ui->tabWidget->setTabVisible( index, bVisible );
}

void NxMainWindow::OnTabChanged( int index )
{
    if ( !ui->tabWidget->currentWidget()->isEnabled() )
    {
        QSignalBlocker blocker( ui->tabWidget );
        ui->tabWidget->setCurrentIndex( m_currentTabIndex );
        return;
    }

    m_currentTabIndex = index;
}

void NxMainWindow::ErrorReported( NxException ex )
{
    HxMsgError( ex.Message(), ex.Where());
}
