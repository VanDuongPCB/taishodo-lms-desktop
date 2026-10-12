#pragma once
#include "QMainWindow"

#include "XMainWindow.h"
namespace Ui
{
    class TransferWindow;
}

class NxTransferWindow : public XMainWindow
{
    Q_OBJECT

public:
    explicit NxTransferWindow( QWidget* parent = nullptr );
    ~NxTransferWindow();

private:
    Ui::TransferWindow* ui;
    void showEvent( QShowEvent* );
    bool eventFilter( QObject* watched, QEvent* event );
    void OnShowModels();
    void OnSelect( const QString& modelName );
    void OnCvWidthChanged( double width );
    void OnTransfer();
    void OnStop();

    void LockUI();
    void UpdateUI();
};

