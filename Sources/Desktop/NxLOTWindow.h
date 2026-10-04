#pragma once
#include "QMainWindow"
#include "QStandardItem"
#include "QLineEdit"
#include "NxLOT.h"
#include "NxModel.h"

#include "NxBadge.h"

namespace Ui
{
    class LotWindow;
}

class NxLOTWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NxLOTWindow( QWidget* parent = 0 );
    ~NxLOTWindow();

private:
    Ui::LotWindow* ui;
    NxLOTPtrMap m_LOTs;
    NxModelPtrMap m_Models;
    NxLOTPtrMap m_lotToSaves;
    NxBadge* m_pBadge = nullptr;
    QLineEdit* m_pSearchTextBox;

    void showEvent( QShowEvent* );

    NxModelPtr FindModel( const QString& name );

    void OnRefresh();
    void OnFilter( const QString& filter = QString() );
    void OnNew();
    void OnDelete();
    void OnEdit( const QModelIndex& index );
    void OnSave();
};

