#pragma once
#include "QDialog"

namespace Ui
{
    class RegisterDialog;
}

class NxRegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NxRegisterDialog( QWidget* parent = nullptr );
    ~NxRegisterDialog();

private:
    Ui::RegisterDialog* ui;
    void OnRegister();
    void OnMakeKeyOrRegister();
};

