#include "NxSettingsWindow.h"
#include "ui_nxsettingswindow.h"
#include "QLineEdit"

#include "NxSettings.h"
#include "NxStopper.h"
#include "NxMarker.h"
#include "NxProfile.h"
#include "NxLogger.h"
#include "NxPLC.h"
#include "NxBarcode.h"
#include "NxLaser.h"
#include "NxMessage.h"

#include "QSerialPort"
#include "QSerialPortInfo"
#include "QFileDialog"

#include "NxFileManager.h"
#include "NxEvent.h"
#include "NxMigration.h"
#include "NxMigrationDialog.h"

NxSettingsWindow::NxSettingsWindow( QWidget* parent ) : QMainWindow( parent ), ui( new Ui::SettingsWindow )
{
    ui->setupUi( this );

    ui->tbvStoppers->setHeaders( { "X","Y" } );

    connect( ui->actionLoad, &QAction::triggered, this, &NxSettingsWindow::OnLoad );
    connect( ui->actionSave, &QAction::triggered, this, &NxSettingsWindow::OnSave );
    connect( ui->actionMigration, &QAction::triggered, this, &NxSettingsWindow::OnMigration );
    connect( ui->btnBrowseData, &QPushButton::clicked, this, &NxSettingsWindow::OnBrowseDataDir );
    connect( ui->btnBrowseIV, &QPushButton::clicked, this, &NxSettingsWindow::OnBrowseIVDir );
    connect( ui->btnEnumCOM, &QPushButton::clicked, this, &NxSettingsWindow::OnEnumerateLaserPorts );
    
}

NxSettingsWindow::~NxSettingsWindow()
{
    delete ui;
}

void NxSettingsWindow::showEvent( QShowEvent* )
{
    OnLoad();
}

bool NxSettingsWindow::eventFilter( QObject* watched, QEvent* event )
{
    return QMainWindow::eventFilter( watched, event );
}

void NxSettingsWindow::Show()
{
    ui->txtAdminPassword->setText( m_regSettings.String( AdminPassword ) );
    ui->txtSuperPassword->setText( m_regSettings.String( SuperPassword ) );
    ui->txtLeaderPassword->setText( m_regSettings.String( LeaderPassword ) );
    ui->txtDatabasePath->setText( m_regSettings.String( DatabaseFilePath ) );
    ui->txtIVDir->setText( m_regSettings.String( IVProgramDir ) );

    ui->cbxLaserPort->setCurrentText( m_regSettings.String( LaserConnPort ) );

    ui->txtPLCIP->setText( m_regSettings.String( PLCConnIPAddress ) );
    ui->spxPLCPort->setValue( m_regSettings.Int( PLCConnPort ) );

    ui->txtRegCvWidth->setText( m_regSettings.String( RegCvWidth) );
    ui->txtRegStopper->setText( m_regSettings.String( RegStopper ) );
    ui->txtRegPassMode->setText( m_regSettings.String( RegPassMode ) );
    ui->txtRegSWReady->setText( m_regSettings.String( RegSoftwareReady ) );
    ui->txtRegTrigger->setText( m_regSettings.String( RegLaserTrigger ) );
    ui->txtRegTriggerConfirm->setText( m_regSettings.String( RegLaserTriggerConfirm ) );
    ui->txtRegStop->setText( m_regSettings.String( RegLaserStop ) );
    ui->txtRegStopConfirm->setText( m_regSettings.String( RegLaserStopConfirm ) );
    ui->txtRegBarCode->setText( m_regSettings.String( RegBarcodeHasData ) );
    ui->txtRegBarcodeConfirm->setText( m_regSettings.String( RegBarcodeConfirm ) );
    ui->txtRegBarcodeData->setText( m_regSettings.String( RegBarcodeData ) );
    ui->txtRegBarcodeOK->setText( m_regSettings.String( RegBarcodeOK ) );
    ui->txtRegBarcodeNG->setText( m_regSettings.String( RegBarcodeNG ) );
    ui->txtRegPrintRes->setText( m_regSettings.String( RegMarkingResult ) );

    ui->tbvStoppers->setRowCount( m_pStoppers.size() );
    int row = 0;
    for ( auto& [index, pStopper] : m_pStoppers )
    {
        ui->tbvStoppers->setText( row, "X", QString::number( pStopper->x, 'f', 2 ) );
        ui->tbvStoppers->setText( row, "Y", QString::number( pStopper->y, 'f', 2 ) );
        row++;
    }
}

void NxSettingsWindow::OnLoad()
{
    m_regSettings.Load();
    m_pStoppers = StopperManager()->GetStoppers();

    Show();
}

void NxSettingsWindow::OnBrowseDataDir()
{
    QFileInfo fi( m_regSettings.String( DatabaseFilePath ) );
    QString dir = QFileDialog::getExistingDirectory( this, "Chọn thư mục dữ liệu", fi.absoluteDir().absolutePath() );
    if ( !dir.isEmpty() )
    {
        ui->txtDatabasePath->setText( dir + "/LMS.db");
    }
}

void NxSettingsWindow::OnBrowseIVDir()
{
    QString dir = QFileDialog::getExistingDirectory( this, "Chọn thư mục chương trình IV", ui->txtIVDir->text() );
    if ( !dir.isEmpty() )
    {
        ui->txtIVDir->setText( dir );

    }
}

void NxSettingsWindow::OnEnumerateLaserPorts()
{
    QList<QSerialPortInfo> portInfos = QSerialPortInfo::availablePorts();
    QStringList portnames;
    for ( auto& portinfo : portInfos )
    {
        portnames.push_back( portinfo.portName() );
    }
    if ( !portnames.contains( m_regSettings.String(LaserConnPort ) ) )
    {
        portnames.push_back( m_regSettings.String( LaserConnPort ) );
    }

    std::sort( portnames.begin(), portnames.end() );
    ui->cbxLaserPort->clear();
    ui->cbxLaserPort->addItem( "" );
    ui->cbxLaserPort->addItems( portnames );

    ui->cbxLaserPort->setCurrentText( m_regSettings.String( LaserConnPort ) );
}

void NxSettingsWindow::OnSave()
{
    m_regSettings.Clear();
    m_regSettings.Set( AdminPassword, ui->txtAdminPassword->text() );
    m_regSettings.Set( SuperPassword, ui->txtSuperPassword->text() );
    m_regSettings.Set( LeaderPassword, ui->txtLeaderPassword->text() );
    m_regSettings.Set( DatabaseFilePath, ui->txtDatabasePath->text() );
    m_regSettings.Set( IVProgramDir, ui->txtIVDir->text() );
    m_regSettings.Set( LaserConnPort, ui->cbxLaserPort->currentText() );

    m_regSettings.Set( PLCConnIPAddress, ui->txtPLCIP->text() );
    m_regSettings.Set( PLCConnPort, ui->spxPLCPort->value() );

    m_regSettings.Set( RegCvWidth, ui->txtRegCvWidth->text() );
    m_regSettings.Set( RegStopper, ui->txtRegStopper->text() );
    m_regSettings.Set( RegPassMode, ui->txtRegPassMode->text() );
    m_regSettings.Set( RegSoftwareReady, ui->txtRegSWReady->text() );

    m_regSettings.Set( RegLaserTrigger, ui->txtRegTrigger->text() );
    m_regSettings.Set( RegLaserTriggerConfirm, ui->txtRegTriggerConfirm->text() );
    m_regSettings.Set( RegLaserStop, ui->txtRegStop->text() );
    m_regSettings.Set( RegLaserStopConfirm, ui->txtRegStopConfirm->text() );
    m_regSettings.Set( RegBarcodeHasData, ui->txtRegBarCode->text() );

    m_regSettings.Set( RegBarcodeConfirm, ui->txtRegBarcodeConfirm->text() );
    m_regSettings.Set( RegBarcodeData, ui->txtRegBarcodeData->text() );
    m_regSettings.Set( RegBarcodeOK, ui->txtRegBarcodeOK->text() );
    m_regSettings.Set( RegBarcodeNG, ui->txtRegBarcodeNG->text() );
    m_regSettings.Set( RegMarkingResult, ui->txtRegPrintRes->text() );
    m_regSettings.Save();
    m_regSettings.Load();

    m_pStoppers.clear();
    for ( int i = 0; i < ui->tbvStoppers->RowCount(); i++ )
    {
        NxStopperPtr pStopper = StopperManager()->Create();
        pStopper->x = ui->tbvStoppers->item( i, 0 )->text().toDouble();
        pStopper->y = ui->tbvStoppers->item( i, 1 )->text().toDouble();
        m_pStoppers[ i + 1 ] = pStopper;
        StopperManager()->Save( i + 1, pStopper );
    }

    LOTManager()->ReloadSetting();
    ModelManager()->ReloadSetting();
    DesignManager()->ReLoadSetting();
    Logger()->ReLoadSetting();
    Marker()->ReLoadSetting();
    PLC()->ReLoadSetting();
    Barcode()->ReLoadSetting();
    Laser()->ReLoadSetting();
    StopperManager()->ReLoadSetting();

    qApp->postEvent( qApp, new NxEvent( NxEvent::eSettingChanged ) );
    HxMsgInfo( "Đã lưu cài đặt !", "Thông báo" );
}

void NxSettingsWindow::OnMigration()
{
    NxMigrationDialog dialog( this );
    dialog.exec();
    OnLoad();
}