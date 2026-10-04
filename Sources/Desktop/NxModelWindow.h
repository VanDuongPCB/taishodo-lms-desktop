#pragma once
#include "QMainWindow"
#include "QLabel"
#include "QLineEdit"
#include "QStandardItem"

#include "NxModel.h"
#include "NxBadge.h"

namespace Ui
{
    class ModelWindow;
}

class NxModelWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NxModelWindow( QWidget* parent = nullptr );
    ~NxModelWindow();
private:
    Ui::ModelWindow* ui;
    NxModelPtrMap m_models;
    NxModelPtr m_pModel;
    NxModelPtrMap m_modelToSave;

    NxBadge* m_pBadge = nullptr;
    QLineEdit* m_pSearchTextBox = nullptr;

    void showEvent( QShowEvent* );

    void ShowModels();
    void ShowModelInfo();
    void ShowMarkPositions();
    void ShowMarkBlocks();
    void ShowComments();
    void ShowBadge();

    void OnRefresh();
    void OnFilter(const QString& filter);
    void OnSelect( const QModelIndex& index );

    void OnNew();
    void OnRemove();
    void OnSave();
    void OnAddParam();
    void OnRemoveParam();

    void OnInfoChanged();
    void OnPositionChanged( QStandardItem* item );
    void OnCommentChanged( QStandardItem* item );
};
