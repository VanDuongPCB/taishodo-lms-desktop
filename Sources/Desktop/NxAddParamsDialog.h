#pragma once
#include "QDialog"

namespace Ui
{
    class AddParamsDialog;
}

class NxAddParamsDialog : public QDialog
{
    Q_OBJECT

public:
    QStringList m_names;
    bool m_bIsApplyAll = false;
    bool m_bIsDefault = false;
    explicit NxAddParamsDialog( QWidget* parent = nullptr );
    ~NxAddParamsDialog();

private slots:
    void on_btnAdd_clicked();

private:
    Ui::AddParamsDialog* ui;
};

