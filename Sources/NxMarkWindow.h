#pragma once
#include "QMainWindow"

#include "NxLOT.h"
#include "NxException.h"

namespace Ui
{
    class MarkWindow;
}

class NxMarkWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NxMarkWindow( QWidget* parent = 0 );
    ~NxMarkWindow();

private:
    Ui::MarkWindow* ui;
    std::vector<NxException> exceptions;
    void showEvent( QShowEvent* );
    bool eventFilter( QObject* watched, QEvent* event );

    void ShowLotInfo();
    void ShowLotStatus();
    void ShowLotBlocks();
    void ShowExceptions();

    void LockUI();
    void UpdateUI();

    void OnException( QJsonObject exData );

    void OnSelect();
    void OnTest();
    void OnRun();
    void OnStop();
};

