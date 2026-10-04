#include "NxLoginDialog.h"
#include "ui_nxlogindialog.h"
#include "NxProtector.h"

NxLoginDialog::NxLoginDialog( QWidget* parent ) : QDialog( parent ), ui( new Ui::LoginDialog )
{
    ui->setupUi( this );
    ui->cbxProfile->setFocus();
    auto setPasswordFocus = [ & ]()
        {
            ui->txtPassword->setFocus();
        };
    connect( ui->cbxProfile, &QComboBox::currentTextChanged, this, setPasswordFocus );
    connect( ui->btnLogin, &QPushButton::clicked, this, &NxLoginDialog::OnLogin );
    setPasswordFocus();
}

NxLoginDialog::~NxLoginDialog()
{
    delete ui;
}

void NxLoginDialog::OnLogin()
{
    QString user = ui->cbxProfile->currentText().trimmed();
    QString pass = ui->txtPassword->text().trimmed();
    if ( Protector()->Login(user, pass) )
    {
        this->close();
        this->setResult( 1 );
    }
}
