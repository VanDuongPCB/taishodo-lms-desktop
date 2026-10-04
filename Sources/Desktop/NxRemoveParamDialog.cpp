#include "NxRemoveParamDialog.h"
#include "ui_NxRemoveParamDialog.h"

NxRemoveParamDialog::NxRemoveParamDialog( QWidget* parent ) : QDialog( parent ), ui( new Ui::RemoveParamDialog )
{
    ui->setupUi( this );
}

NxRemoveParamDialog::~NxRemoveParamDialog()
{
    delete ui;
}

void NxRemoveParamDialog::setParams( QStringList names )
{
    ui->txtParamNames->setPlainText( names.join( ',' ) );
}

void NxRemoveParamDialog::on_btnRemove_clicked()
{
    isApplyAll = ui->chxApplyAll->isChecked();
    this->close();
    this->setResult( 1 );
}

