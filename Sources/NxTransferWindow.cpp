#include "NxTransferWindow.h"
#include "ui_nxtransferwindow.h"
#include "NxModel.h"
#include "NxMarker.h"
#include "NxPLC.h"
#include "NxMessage.h"
#include "NxException.h"
#include "NxSystemReport.h"
#include "NxEvent.h"

NxTransferWindow::NxTransferWindow( QWidget* parent ) : QMainWindow( parent ), ui( new Ui::TransferWindow )
{
    ui->setupUi( this );
    qApp->installEventFilter( this );
    connect( ui->cbxModel, &QComboBox::currentTextChanged, this, &NxTransferWindow::OnSelect );
    connect( ui->spxCvWidth, &QDoubleSpinBox::valueChanged, this, &NxTransferWindow::OnCvWidthChanged );
    connect( ui->btnPass, &QToolButton::clicked, this, &NxTransferWindow::OnTransfer );
    connect( ui->btnStop, &QToolButton::clicked, this, &NxTransferWindow::OnStop );
}

NxTransferWindow::~NxTransferWindow()
{
    delete ui;
}

void NxTransferWindow::showEvent( QShowEvent* )
{
    OnShowModels();
    UpdateUI();
}

bool NxTransferWindow::eventFilter( QObject* watched, QEvent* event )
{
    NxEvent* NxEvent( nullptr );
    NxEvent::Type type;
    if ( !NxEvent::IsCustomEvent( event, NxEvent, type ) )
        return QMainWindow::eventFilter( watched, event );

    switch ( type )
    {
    case NxEvent::eDesignAdded:
    case NxEvent::eDesignDeleted:
    case NxEvent::eDesignChanged:
    case NxEvent::eModelAdded:
    case NxEvent::eModelDeleted:
    case NxEvent::eModelChanged:
    case NxEvent::eLOTAdded:
    case NxEvent::eLOTDeleted:
    case NxEvent::eLOTChanged:
    case NxEvent::eSettingChanged:
        OnShowModels();
        break;
    case NxEvent::eMarkerSetupChanged:
    case NxEvent::eMarkerGoFree:
    case NxEvent::eMarkerGoTransfer:
    case NxEvent::eMarkerGoMark:
        UpdateUI();
        break;
    case NxEvent::eMarkerStopped:
        break;
    case NxEvent::eMarkerGoError:
        break;
    default:
        break;
    }

    return QMainWindow::eventFilter( watched, event );
}

void NxTransferWindow::OnShowModels()
{
    QSignalBlocker blocker( ui->cbxModel );
    QString old = ui->cbxModel->currentText();
    ui->cbxModel->clear();
    auto Models = ModelManager()->GetModels();
    for ( auto& [name, md] : Models )
    {
        ui->cbxModel->addItem( md->Name() );
    }
    ui->cbxModel->setCurrentText( old );
}

void NxTransferWindow::OnSelect( const QString& modelName )
{
    if ( Marker()->Setup( NxMarker::eModeTransfer, modelName ) )
    {
        NxModelPtr pModel = Marker()->Model();
        if ( pModel )
        {
            QSignalBlocker blocker( ui->spxCvWidth );
            ui->spxCvWidth->setValue( pModel->CvWidth() );
        }
    }
    UpdateUI();
}

void NxTransferWindow::OnCvWidthChanged( double width )
{
    try
    {
        PLC()->SetCvWidth( ui->spxCvWidth->value() );
    }
    catch ( NxException ex )
    {
        HxMsgError( ex.Message(), ex.Where() );
    }
}

void NxTransferWindow::OnTransfer()
{
    LockUI();
    Marker()->Transfer();
}

void NxTransferWindow::OnStop()
{
    LockUI();
    Marker()->Pause();
}

void NxTransferWindow::LockUI()
{
    ui->cbxModel->setEnabled( false );
    ui->spxCvWidth->setEnabled( false );
    ui->btnPass->setEnabled( false );
    ui->btnStop->setEnabled( false );
}

void NxTransferWindow::UpdateUI()
{
    auto pModel = Marker()->Model();
    auto state = Marker()->GetState();

    ui->cbxModel->setEnabled( state == NxMarker::eOnFree );
    ui->spxCvWidth->setEnabled( state == NxMarker::eOnFree );
    ui->btnPass->setEnabled( state == NxMarker::eOnFree && pModel != nullptr );
    ui->btnStop->setEnabled( state == NxMarker::eOnTransfering );
}