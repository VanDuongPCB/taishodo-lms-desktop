#pragma once
#include "QDialog"

namespace Ui
{
    class RemoveParamDialog;
}

class NxRemoveParamDialog : public QDialog
{
    Q_OBJECT

public:
    bool isApplyAll = false;
    QStringList names;
    explicit NxRemoveParamDialog( QWidget* parent = nullptr );
    ~NxRemoveParamDialog();
    void setParams( QStringList names );
private slots:
    void on_btnRemove_clicked();

private:
    Ui::RemoveParamDialog* ui;
};

