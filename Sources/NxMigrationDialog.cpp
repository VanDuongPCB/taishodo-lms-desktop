#include "NxMigrationDialog.h"
#include "ui_nxmigrationdialog.h"

#include "QFileDialog"

#include "NxMigration.h"


NxMigrationDialog::NxMigrationDialog( QWidget* parent ) : QDialog( parent ), ui( new Ui::MigrationDialog )
{
    ui->setupUi( this );

    ui->txtDBFile->setText( m_settings.String( DatabaseFilePath ) );

    connect( ui->btnBrowseDataDir, &QPushButton::clicked, this, &NxMigrationDialog::OnBrowseOldDataDir );
    connect( ui->btnMigrateLOTs, &QPushButton::clicked, this, &NxMigrationDialog::OnMigrationLOTs );
    connect( ui->btnMigrateModels, &QPushButton::clicked, this, &NxMigrationDialog::OnMigrationModels );
    connect( ui->btnMigrateDesigns, &QPushButton::clicked, this, &NxMigrationDialog::OnMigrationDesigns );
    connect( ui->btnMigrateStoppers, &QPushButton::clicked, this, &NxMigrationDialog::OnMigrationStoppers );
}
NxMigrationDialog::~NxMigrationDialog()
{
    delete ui;
}

void NxMigrationDialog::OnBrowseOldDataDir()
{
    QString dir = QFileDialog::getExistingDirectory( this, tr("Chọn thư mục chương trình IV"), ui->txtRootDir->text() );
    if ( !dir.isEmpty() )
    {
        ui->txtRootDir->setText( dir );
        ui->txtLOTDir->setText( dir + "/data/LOTS" );
        ui->txtModelDir->setText( dir + "/data/MODELS" );
        ui->txtDesinDir->setText( dir + "/data/DESIGNS" );
        ui->txtPrintLogDir->setText( dir + "/data/PRINT-LOGS" );
        ui->txtRePrintLogDir->setText( dir + "/data/REPRINT-LOGS" );
        ui->txtBarcodeDir->setText( dir + "/data/BARCODE-LOGS" );
        ui->txtStopperFile->setText( dir + "/settings/stoppers.json" );
    }
}

void NxMigrationDialog::OnMigrationLOTs()
{
    MigrateLOTs( ui->txtLOTDir->text() );
}

void NxMigrationDialog::OnMigrationModels()
{
    MigrateModels( ui->txtModelDir->text() );
}

void NxMigrationDialog::OnMigrationDesigns()
{
    MigrateDesigns( ui->txtDesinDir->text() );
}

void NxMigrationDialog::OnMigrationPrintLogs()
{
    MigratePrintLogs( ui->txtPrintLogDir->text() );
}

void NxMigrationDialog::OnMigrationRePrintLogs()
{
    MigrateRePrintLogs( ui->txtRePrintLogDir->text() );
}

void NxMigrationDialog::OnMigrationBarcodes()
{
    MigrateBarcodes( ui->txtBarcodeDir->text() );
}

void NxMigrationDialog::OnMigrationStoppers()
{
    MigrateStoppers( ui->txtStopperFile->text() );
}