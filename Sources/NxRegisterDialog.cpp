#include "NxRegisterDialog.h"
#include "ui_nxregisterdialog.h"
#include "NxLicense.h"
#include "NxMessage.h"

NxRegisterDialog::NxRegisterDialog( QWidget* parent ) : QDialog( parent ), ui( new Ui::RegisterDialog )
{
    ui->setupUi( this );
    ui->txtID->setText( Licensing()->ID() );
    ui->txtVersion->setText(Licensing()->GetVersion());
    ui->txtKey->setFocus();
    connect(ui->btnRegister, &QPushButton::clicked, this, &NxRegisterDialog::OnRegister);
    connect(ui->txtKey, &QLineEdit::returnPressed, this, &NxRegisterDialog::OnMakeKeyOrRegister);
}

NxRegisterDialog::~NxRegisterDialog()
{
    delete ui;
}

void NxRegisterDialog::OnRegister()
{
    QString keyIn = ui->txtKey->text().trimmed();
    if ( Licensing()->RegisterKey( keyIn ) )
    {
        this->close();
        this->setResult( 1 );
    }
    else
    {
        HxMsgError(tr("Key không hợp lệ!"));
    }
}


void NxRegisterDialog::OnMakeKeyOrRegister()
{
    QString txt = ui->txtKey->text().trimmed().toUpper();
    if ( txt.startsWith( "--KEYGEN=" ) )
    {
        txt = txt.replace( "--KEYGEN=", "" );
        ui->txtKey->setText( Licensing()->KeyFromId( txt ) );
    }
}

