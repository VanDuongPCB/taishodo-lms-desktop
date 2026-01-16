#pragma once
#include <QDialog>

#include "NxProfile.h"

namespace Ui
{
    class LoginDialog;
}

class NxLoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NxLoginDialog( QWidget* parent = 0 );
    ~NxLoginDialog();

private:
    Ui::LoginDialog* ui;
    void OnLogin();
};

