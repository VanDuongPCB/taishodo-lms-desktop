#include "NxMainWindow.h"
#include "ui_nxmainwindow.h"

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

NxMainWindow::NxMainWindow( QWidget* parent ) : QMainWindow( parent ), ui( new Ui::MainWindow )
{
    ui->setupUi( this );

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

#ifndef DEBUG_MODE
    OnLockUI();
#endif

    Theme()->SetTheme( "Default" );
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
        return QMainWindow::eventFilter( watched, event );

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

    return QMainWindow::eventFilter( watched, event );
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
    ui->tabWidget->setTabText( 0, tr( "Đăng nhập" ) );
    ui->tabWidget->setTabVisible( 3, false );
    ui->tabWidget->setTabVisible( 4, false );
    ui->tabWidget->setTabVisible( 5, false );
    //ui->tabWidget->setTabVisible( 6, false );
    ui->tabWidget->setTabVisible( 7, false );
    ui->tabWidget->setTabVisible( 8, false );
    //ui->tabWidget->setTabVisible( 9, false );
}

void NxMainWindow::OnUnLockUIForLeader()
{
    OnUnLockUIForAdmin();
    ui->tabWidget->setTabVisible( 4, false );
    ui->tabWidget->setTabVisible( 5, false );
    ui->tabWidget->setTabVisible( 7, false );
    ui->tabWidget->setTabVisible( 8, false );
}

void NxMainWindow::OnUnLockUIForSuper()
{
    OnUnLockUIForAdmin();
    ui->tabWidget->setTabVisible( 7, false );
    ui->tabWidget->setTabVisible( 8, false );
}

void NxMainWindow::OnUnLockUIForAdmin()
{
    ui->tabWidget->setTabText( 0, tr( "Đăng xuất" ) );
    for ( int i = 0; i < ui->tabWidget->count(); i++ )
        ui->tabWidget->setTabVisible( i, true );
}

void NxMainWindow::OnTabChanged( int index )
{
    if ( !ui->tabWidget->currentWidget()->isEnabled() || index == 0 )
    {
        QSignalBlocker blocker( ui->tabWidget );
        ui->tabWidget->setCurrentIndex( m_currentTabIndex );
        if ( index == 0 )
            OnLoginOrLogout();
        return;
    }

    m_currentTabIndex = index;
}

void NxMainWindow::ErrorReported( NxException ex )
{
    HxMsgError( ex.Message(), ex.Where());
}
