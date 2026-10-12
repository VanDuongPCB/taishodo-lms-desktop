#pragma once
#include "QDialog"

#include "XDialog.h"
namespace Ui
{
    class RegisterDialog;
}

class NxRegisterDialog : public XDialog
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

