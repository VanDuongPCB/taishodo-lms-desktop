#include "NxControlWindow.h"
#include "ui_NxControlWindow.h"
#include "NxMarker.h"
#include "NxLaser.h"
#include "NxException.h"
#include "NxSystemReport.h"
#include "NxMessage.h"
#include "QFile"
#include "QMap"
#include "NxPLC.h"
#include "NxBarcode.h"

NxControlWindow::NxControlWindow( QWidget* parent ) : QMainWindow( parent ), ui( new Ui::ControlWindow )
{
    ui->setupUi( this );
    on_btnBlockClear_clicked();
}

NxControlWindow::~NxControlWindow()
{
    delete ui;
}

void NxControlWindow::on_btnBarcodeRead_clicked()
{
    ui->txtBarcodeData->setText( "" );
    try
    {
        bool bit = Barcode()->IsHasData();
        if ( bit )
        {
            Barcode()->Clear();
            QString barcode = Barcode()->Read();
            ui->txtBarcodeData->setText( barcode );
        }
    }
    catch ( NxException ex )
    {
        HxMsgError( ex.Message() );
    }
}

void NxControlWindow::on_spxCvWidth_valueChanged( double arg1 )
{
    try
    {
        PLC()->SetCvWidth( arg1 );
    }
    catch ( NxException ex )
    {
        HxMsgError( ex.Message() );
    }
}

void NxControlWindow::on_cbxStopper_currentIndexChanged( int index )
{
    try
    {
        PLC()->SetStopper( index );
    }
    catch ( NxException ex )
    {
        HxMsgError( ex.Message() );
    }
}

void NxControlWindow::on_btnBlockClear_clicked()
{
    if ( ui->tbvMarkBlocks->headers.empty() )
    {
        ui->tbvMarkBlocks->setHeaders( { "Block","Giá trị" } );
        int rows = 16;
        ui->tbvMarkBlocks->setRowCount( rows );
        for ( int row = 0; row < rows; row++ )
        {
            ui->tbvMarkBlocks->setText( row, 0, QString::number( row ).rightJustified( 3, '0' ) );
            ui->tbvMarkBlocks->item( row, 0 )->setFlags( ui->tbvMarkBlocks->item( row, 0 )->flags() & ~Qt::ItemIsEditable );
            ui->tbvMarkBlocks->setText( row, 1, "" );
        }
    }
}

void NxControlWindow::on_btnMark_clicked()
{
    QString program = QString::number( ui->spxProgram->value() ).rightJustified( 4, '0' );
    int stopper = ui->cbxStopper->currentIndex() + 1;
    QMap<QString, QString> data;
    int rows = 16;
    for ( int row = 0; row < rows; row++ )
    {
        if ( ui->tbvMarkBlocks->item( row, 1 ) == nullptr ) continue;
        QString text = ui->tbvMarkBlocks->item( row, 1 )->text().trimmed();
        if ( text.length() < 1 ) continue;
        QString block = ui->tbvMarkBlocks->item( row, 0 )->text();
        data.insert( block, text );
    }

    try
    {
        if ( ui->chxChangeProgram->isChecked() )
        {
            Laser()->SetProgram( program );
        }
    }
    catch ( NxException ex )
    {
        HxMsgError( ex.Message() );
    }

    if ( ui->chxChangeProgram->isChecked() )
    {
        try
        {
            Laser()->SetProgram( program );
        }
        catch ( NxException ex )
        {
            HxMsgError( ex.Message() );
        }
    }

    if ( ui->chxChangePos->isChecked() )
    {
        NxPosition pos;
        pos.x = ui->spxMarkX->value();
        pos.y = ui->spxMarkY->value();
        pos.angle = ui->spxMarkAngle->value();
        NxDesignPtr pDesign= DesignManager()->Create();
        NxStopperPtr pStopper = StopperManager()->GetStopper( stopper );
        Laser()->SetupPosition( program, pos, pDesign, pStopper );
    }

    if ( ui->chxChangeContent->isChecked() )
    {
        std::map<int, QString> data;
        int rows = 16;
        for ( int row = 0; row < rows; row++ )
        {
            if ( ui->tbvMarkBlocks->item( row, 1 ) == nullptr ) 
                continue;
            QString text = ui->tbvMarkBlocks->item( row, 1 )->text().trimmed();
            if ( text.length() < 1 ) 
                continue;
            int block = ui->tbvMarkBlocks->item( row, 0 )->text().toInt();
            data[ block ] = text;
        }
        Laser()->SetupBlockData( program, data );
    }

    if ( ui->chxPrint->isChecked() )
    {
        Laser()->Burn();
    }



    //    QString program = QString::number(ui->spxProgram->value()).rightJustified(4,'0');
    //    int stopper = ui->cbxStopper->currentIndex() + 1;
    //    QMap<QString, QString> data;
    //    int rows = 16;
    //    for(int row = 0; row < rows; row++){
    //        if(ui->tbvMarkBlocks->item(row,1) == nullptr) continue;
    //        QString text = ui->tbvMarkBlocks->item(row,1)->text().trimmed();
    //        if(text.length() < 1) continue;
    //        QString block = ui->tbvMarkBlocks->item(row,0)->text();
    //        data.insert(block, text);
    //    }

    //    try{
    //        if(ui->chxChangeProgram->isChecked()){
    //            GetLaserMachine()->setProgram(program);
    //        }
    //    }
    //    catch(Exception ex){
    //        Message::error(ex.message());
    //    }




    //    if(ui->chxChangeProgram->isChecked()){
    ////        Controller::instance()->setupLaserProgram(program);
    //        try{
    //            GetLaserMachine()->setProgram(program);
    //        }
    //        catch(std::exception ex){
    //            std::cout << ex.what() << std::endl;
    //        }
    //    }

    //    if(ui->chxChangePos->isChecked()){
    //        Position pos;
    //        pos.x = ui->spxMarkX->value();
    //        pos.y = ui->spxMarkY->value();
    //        pos.angle = ui->spxMarkAngle->value();
    //        Marker::instance()->setupLaserPos(program,stopper, pos,26,7);
    //    }

    //    if(ui->chxChangeContent->isChecked()){
    //        QMap<QString, QString> data;
    //        int rows = 16;
    //        for(int row = 0; row < rows; row++){
    //            if(ui->tbvMarkBlocks->item(row,1) == nullptr) continue;
    //            QString text = ui->tbvMarkBlocks->item(row,1)->text().trimmed();
    //            if(text.length() < 1) continue;
    //            QString block = ui->tbvMarkBlocks->item(row,0)->text();
    //            data.insert(block, text);
    //        }
    //        Marker::instance()->setupLaserBlocks(program, data);
    //    }

    //    if(ui->chxPrint->isChecked()){
    //        Marker::instance()->setupLaserOn();
    //    }

}
