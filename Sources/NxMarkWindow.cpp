#include "NxMarkWindow.h"
#include "ui_NxMarkWindow.h"


#include "NxMarker.h"
#include "NxMessage.h"
#include "NxException.h"
#include "NxSystemReport.h"
#include "NxLaser.h"
#include "NxPLC.h"
#include "NxEvent.h"

#include "NxConvert.h"
#include "NxDataGenerator.h"

#include "NxLOTSelector.h"

NxMarkWindow::NxMarkWindow( QWidget* parent ) : QMainWindow( parent ), ui( new Ui::MarkWindow )
{
    ui->setupUi( this );
    qApp->installEventFilter( this );

    ui->tbvBlocks->setHeaders( { "Block","Dữ liệu" } );
    ui->tbvErrors->setHeaders( { "Thời gian","Từ","Lỗi" } );

    connect( ui->actionSelect, &QAction::triggered, this, &NxMarkWindow::OnSelect );
    connect( ui->actionMark, &QAction::triggered, this, &NxMarkWindow::OnTest );
    connect( ui->actionRun, &QAction::triggered, this, &NxMarkWindow::OnRun );
    connect( ui->actionStop, &QAction::triggered, this, &NxMarkWindow::OnStop );
}

NxMarkWindow::~NxMarkWindow()
{
    delete ui;
}

void NxMarkWindow::showEvent( QShowEvent* )
{
    ShowLotInfo();
    ShowLotBlocks();
    ShowLotStatus();
}

bool NxMarkWindow::eventFilter( QObject* watched, QEvent* event )
{
    NxEvent* NxEvent( nullptr );
    NxEvent::Type type;
    if ( !NxEvent::IsCustomEvent( event, NxEvent, type ) )
        return QMainWindow::eventFilter( watched, event );

    switch ( type )
    {
    case NxEvent::eMarkerSetupChanged:
    {
        ShowLotInfo();
        ShowLotBlocks();
        ShowLotStatus();
        UpdateUI();
    }
        break;
    case NxEvent::eMarkerGoFree:
    case NxEvent::eMarkerGoTransfer:
    case NxEvent::eMarkerGoTest:
    case NxEvent::eMarkerGoMark:
        UpdateUI();
        break;
    case NxEvent::eMarkerMarked:
        ShowLotBlocks();
        ShowLotStatus();
        break;
    case NxEvent::eMarkerGoFinish:
        UpdateUI();
        break;
    case NxEvent::eDatabaseError:
    case NxEvent::eMarkerGoError:
    {
        QVariant data = NxEvent->Data();
        QJsonObject obj = data.toJsonObject();
        OnException( obj );
    }
        break;
    case NxEvent::eMarkerStopped:
        break;
    default:
        break;
    }
    return QMainWindow::eventFilter( watched, event );
}

void NxMarkWindow::ShowLotInfo()
{
    NxLOTPtr pLOT = Marker()->LOT();
    if ( !pLOT )
    {
        ui->lblLotName->setText( "Chưa chọn lot..." );
        ui->lblCounterStart->setText( "Chưa chọn lot..." );
        ui->lblSerialStart->setText( "Chưa chọn lot..." );
        ui->lblSerialEnd->setText( "Chưa chọn lot..." );
        ui->lblQuantity->setText( "Chưa chọn lot..." );
        ui->lblStatus->setText( "Chưa chọn lot..." );
        ui->lblModelName->setText( "Chưa chọn lot..." );
        ui->lblModelCode->setText( "Chưa chọn lot..." );
        ui->lblDesign->setText( "Chưa chọn lot..." );
        ui->lblIVProgram->setText( "Chưa chọn lot..." );
    }
    else
    {
        ui->lblLotName->setText( pLOT->Name() );
        ui->lblCounterStart->setText( pLOT->CounterStart() );
        ui->lblSerialStart->setText( pLOT->MACStart() );
        ui->lblSerialEnd->setText( pLOT->MACEnd() );
        ui->lblQuantity->setText( QString::number( pLOT->Quantity() ) );
        ui->lblStatus->setText( ProductStatusToString( pLOT->Status() ).toUpper() );
        ui->lblModelName->setText( pLOT->Model() );
        auto model = ModelManager()->GetModel( pLOT->Model() );
        if ( model != nullptr )
        {
            ui->lblModelCode->setText( model->Code() );
            ui->lblDesign->setText( model->Design() );
            ui->lblIVProgram->setText( model->IVProgram() );
        }
        else
        {
            ui->lblModelCode->setText( "Không xác định..." );
            ui->lblDesign->setText( "Không xác định..." );
            ui->lblIVProgram->setText( "Không xác định..." );
        }
    }
}

void NxMarkWindow::ShowLotStatus()
{
    NxLOTPtr pLOT = Marker()->LOT();
    if ( !pLOT )
    {
        ui->pgbProgress->setMaximum( 1000 );
        ui->pgbProgress->setValue( 0 );
        ui->pgbProgress->setFormat( "" );
        ui->lblStatus->setText( "KHÔNG XÁC ĐỊNH" );
        ui->lblStatus->setStyleSheet( "color:#888" );
    }
    else
    {
        ui->pgbProgress->setMaximum( pLOT->Quantity() );
        ui->pgbProgress->setValue( pLOT->Progress() );
        ui->pgbProgress->setFormat( "%v/%m" );
        ui->lblStatus->setText( ProductStatusToString( pLOT->Status() ).toUpper() );

        switch ( pLOT->Status() )
        {
        case NxLOT::ePending:
            ui->lblStatus->setStyleSheet( "color:#888" );
            break;
        case NxLOT::eProduct:
            ui->lblStatus->setStyleSheet( "color:#f80" );
            break;
        case NxLOT::eCompleted:
            ui->lblStatus->setStyleSheet( "color:#080" );
            break;
        default:
            break;
        }
    }
}

void NxMarkWindow::ShowLotBlocks()
{
    ui->tbvBlocks->setRowCount( 0 );
    auto pLOT = Marker()->LOT();
    auto pModel = Marker()->Model();
    auto pDesign = Marker()->Design();
    if ( !pLOT || !pModel || !pDesign )
        return;

    int codeIndex = pDesign->IndexOfBlockCode();

    std::map<int, QString> blockdatas = GenMarkData( pDesign, pLOT, pModel, -1 );
    ui->tbvBlocks->setRowCount( blockdatas.size() );
    int row = 0;
    for ( auto& [index, data] : blockdatas )
    {
        ui->tbvBlocks->setText( row, "Block", QString::number( index ).rightJustified( 3, '0' ) );
        ui->tbvBlocks->setText( row, "Dữ liệu", data );
        if ( index == codeIndex )
            ui->lblBarcode->setText( data );
        row++;
    }

    ui->lblFormat->setText( "" );
    if ( codeIndex > 0 )
    {
        ui->lblFormat->setText( pDesign->Block( codeIndex ).data );
    }
}

void NxMarkWindow::ShowExceptions()
{
    int rows = ( int )exceptions.size();
    ui->tbvErrors->setRowCount( rows );
    for ( int row = 0; row < rows; row++ )
    {
        ui->tbvErrors->setText( row, 0, exceptions[ row ].Time() );
        ui->tbvErrors->setText( row, 1, exceptions[ row ].Where() );
        ui->tbvErrors->setText( row, 2, exceptions[ row ].Message().replace("\n", ", "));
    }
    ui->tbvErrors->scrollToBottom();
}

void NxMarkWindow::LockUI()
{
    ui->actionSelect->setEnabled( false );
    ui->actionMark->setEnabled( false );
    ui->actionRun->setEnabled( false );
    ui->actionStop->setEnabled( false );
}

void NxMarkWindow::UpdateUI()
{
    auto pLOT = Marker()->LOT();
    auto state = Marker()->GetState();

    ui->actionSelect->setEnabled( state == NxMarker::eOnFree || state == NxMarker::eOnFinish );
    ui->actionMark->setEnabled( pLOT != nullptr && state == NxMarker::eOnFree );
    ui->actionRun->setEnabled( pLOT != nullptr && state == NxMarker::eOnFree );
    ui->actionStop->setEnabled( state == NxMarker::eOnMarking );
}

void NxMarkWindow::OnException( QJsonObject exData )
{
    int row = ui->tbvErrors->RowCount();
    ui->tbvErrors->setRowCount( row + 1 );
    ui->tbvErrors->setText( row, 0, exData.value( "time" ).toString() );
    ui->tbvErrors->setText( row, 1, exData.value( "where" ).toString() );
    ui->tbvErrors->setText( row, 2, exData.value( "message" ).toString().replace( "\n", "" ) );
}

//void NxMarkWindow::handleException( NxException ex )
//{
//    exceptions.push_back( ex );
//    ShowExceptions();
//}
//
//void NxMarkWindow::on_actionMark_triggered()
//{
//    //try
//    //{
//    //    NxMarker::instance()->Mark( true );
//    //    HxMsgInfo( "Đã khắc xong !", "In test" );
//    //}
//    //catch ( NxException ex )
//    //{
//    //    //        Message::error(ex.message);
//    //    //ex.where = "In test";
//    //    GetSystemReport()->Report( ex );
//    //}
//}

void NxMarkWindow::OnSelect()
{
    if ( NxLOTSelector( this ).exec() )
    {
        ShowLotInfo();
        ShowLotBlocks();
        ShowLotStatus();
    }
}

void NxMarkWindow::OnTest()
{
    LockUI();
    Marker()->Test();
}

void NxMarkWindow::OnRun()
{
    LockUI();
    Marker()->Mark();
}

void NxMarkWindow::OnStop()
{
    LockUI();
    Marker()->Pause();
}