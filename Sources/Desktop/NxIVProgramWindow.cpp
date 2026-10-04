#include "NxIVProgramWindow.h"
#include "ui_NxIVProgramWindow.h"

#include "NxIVProgram.h"

NxIVProgramWindow::NxIVProgramWindow( QWidget* parent ) : QMainWindow( parent ), ui( new Ui::IVWindow )
{
    ui->setupUi( this );

    ui->tableView->setHeaders( { "Chương trình","Dung lượng", "Sửa đổi", "Mô tả" } );
    ui->tableView->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );

    connect( ui->actionLoad, &QAction::triggered, this, &NxIVProgramWindow::OnLoad );
    OnLoad();
}

NxIVProgramWindow::~NxIVProgramWindow()
{
    delete ui;
}


void NxIVProgramWindow::OnLoad()
{
    auto programs = IVProgram()->Get();
    ui->tableView->setRowCount( programs.size() );
    int row = 0;
    for ( auto& program : programs )
    {
        ui->tableView->setText( row, 0, program.FilePath );
        ui->tableView->setText( row, 1, QString( "%1 MB" ).arg( program.FileSize / 1024.0 / 1024.0 ) );
        ui->tableView->setText( row, 2, program.LastTimeModified.toString( "yyyy-MM-dd" ) );
        row++;
    }
}