#pragma once
#include "QDialog"
#include "NxModel.h"

namespace Ui
{
    class NewModelDialog;
}

class NxNewModelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NxNewModelDialog( NxModelPtrMap models, NxModelPtrMap modelToSaves, QWidget* parent = 0 );
    ~NxNewModelDialog();
    NxModelPtr GetModel() const;
    NxModelPtrMap m_models;
    NxModelPtrMap m_modelToSaves;
    NxModelPtr m_pModel;
private slots:
    void OnCreate();

private:
    Ui::NewModelDialog* ui;
    bool checkInputs();
};

