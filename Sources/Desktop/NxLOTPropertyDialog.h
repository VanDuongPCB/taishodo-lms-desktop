#pragma once
#include "QDialog"
#include "XDialog.h"
#include "QStandardItem"
#include "NxLOT.h"
#include "NxModel.h"
#include "NxDesign.h"

namespace Ui
{
    class LotPropertyDialog;
}

class NxLOTPropertyDialog : public XDialog
{
    Q_OBJECT

public:
    explicit NxLOTPropertyDialog( NxLOTPtr pLOT, NxLOTPtrMap LOTs, QWidget* parent = 0 );
    ~NxLOTPropertyDialog();
    NxLOTPtr GetLOT() const;

private:
    Ui::LotPropertyDialog* ui;

    bool m_bIsCreate;
    NxLOTPtrMap m_LOTs;
    NxLOTPtr m_pLOT;
    NxModelPtr m_pModel;
    NxDesignPtr m_pDesign;

    void OnInit( NxLOTPtr pLOT );


    void ShowInfo();
    void ShowPrintLo();
    void ShowParams();
    void ShowBlocks();

    bool CheckLotName();
    bool CheckSeriRange();
    bool CheckMacs();
    bool CheckModelInfo();
    bool CheckBlocks();

    void GetInputs();
    bool CheckInputs();
    void OnInfoChanged();
    void OnModelChanged();
    void OnParamChanged( QStandardItem* item );
    void OnApply();
};

