#include "NxLOTSelector.h"
#include "ui_nxlotselector.h"
#include "NxLOT.h"
#include "NxConvert.h"
#include "NxMarker.h"

NxLOTSelector::NxLOTSelector( QWidget* parent ) : QDialog( parent ), ui( new Ui::LOTSelector )
{
    ui->setupUi( this );

    ui->tbvLOTs->setHeaders( { "Tên","Kiểu in","Model","MAC đầu","MAC cuối","Sản lượng","Tiến độ","Trạng thái"});
    ui->tbvLOTs->setColumnWidth( 0, 200 );
    ui->tbvLOTs->setColumnWidth( 2, 250 );

    connect( ui->tbvLOTs, &NxTableView::doubleClicked, this, &NxLOTSelector::OnSelected );
    OnShow();
}

NxLOTSelector::~NxLOTSelector()
{
}

NxModelPtr NxLOTSelector::FindModel( const QString& name )
{
    for ( auto& [code, model] : m_Models )
    {
        if ( code == name || model->Name() == name )
        {
            return model;
        }
    }
    return nullptr;
}

void NxLOTSelector::OnShow()
{
    m_LOTs = LOTManager()->GetLOTs( QDate::currentDate() );
    m_Models = ModelManager()->GetModels();

    ui->tbvLOTs->setRowCount( m_LOTs.size() );
    int row = 0;
    for ( auto& [name, pLOT] : m_LOTs )
    {
        auto status = pLOT->Status();
        QString statusName = ProductStatusToString( status );
        QColor statusColor = ProductStatusToColor( status );

        ui->tbvLOTs->setText( row, "Tên", pLOT->Name() );
        
        auto pModel = FindModel( pLOT->Model() );
        if ( pModel && pModel->IsPrintLo() )
            ui->tbvLOTs->setText( row, "Kiểu in", "In mã lô" );
        else if ( pLOT->IsRePrint() )
            ui->tbvLOTs->setText( row, "Kiểu in", "In lại" );
        else
            ui->tbvLOTs->setText( row, "Kiểu in", "" );

        ui->tbvLOTs->setText( row, "Model", pLOT->Model() );
        ui->tbvLOTs->setText( row, "Sản lượng", QString::number( pLOT->Quantity() ) );
        ui->tbvLOTs->setText( row, "Seri đầu", pLOT->CounterStart() );
        ui->tbvLOTs->setText( row, "Seri cuối", pLOT->CounterEnd() );
        ui->tbvLOTs->setText( row, "Tiến độ", QString::number( pLOT->Progress() ) + "/" + QString::number( pLOT->Quantity() ) );
        ui->tbvLOTs->setText( row, "MAC đầu", pLOT->MACStart() );
        ui->tbvLOTs->setText( row, "MAC cuối", pLOT->MACEnd() );
        ui->tbvLOTs->setText( row, "Trạng thái", statusName );

        for ( int col = 0; col < ui->tbvLOTs->dataTable()->columnCount(); col++ )
        {
            ui->tbvLOTs->item( row, col )->setBackground( statusColor );
        }

        row++;
    }
}

void NxLOTSelector::OnSelected( const QModelIndex& index )
{
    int row = index.row();
    QString lotName = ui->tbvLOTs->item( row, 0 )->text();
    if ( Marker()->Setup( NxMarker::eModeMarking, lotName ) )
    {
        close();
        setResult( 1 );
    }
}