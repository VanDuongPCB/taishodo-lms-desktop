#pragma once
#include <QDialog>

#include "XDialog.h"
#include "NxProfile.h"

namespace Ui
{
    class LoginDialog;
}

class NxLoginDialog : public XDialog
{
    Q_OBJECT

public:
    explicit NxLoginDialog( QWidget* parent = 0 );
    ~NxLoginDialog();

private:
    Ui::LoginDialog* ui;
    void OnLogin();
};

