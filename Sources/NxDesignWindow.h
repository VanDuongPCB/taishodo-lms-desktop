#pragma once
#include "QMainWindow"
#include "QLabel"
#include "QStandardItem"

#include "NxDesign.h"
#include "NxBadge.h"

namespace Ui
{
    class DesignWindow;
}

class NxDesignWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NxDesignWindow( QWidget* parent = 0 );
    ~NxDesignWindow();

private:
    Ui::DesignWindow* ui;
    QLabel* m_pLblMessage = nullptr;
    NxBadge* m_pBadge = nullptr;

    NxDesignPtrMap m_designs;
    NxDesignPtr m_pDesign;
    NxDesignPtrMap m_designChanges;

    void showEvent( QShowEvent* );
    void ShowDesigns();
    void ShowBlocks();
    void ShowParams();


    void OnSelected( const QModelIndex& index );
    void OnSizeChanged( QStandardItem* item );
    void OnBlockChanged( QStandardItem* item );
    void OnInsertParam( const QModelIndex& index );
    void OnRefresh();
    void OnSave();

};

