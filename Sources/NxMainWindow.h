#pragma once
#include <QMainWindow>
#include "QLabel"
#include "NxException.h"
#include "NxLOT.h"

#include "NxMarkWindow.h"
#include "NxSettingsWindow.h"
#include "NxTransferWindow.h"
#include "NxControlWindow.h"
#include "NxModelWindow.h"
#include "NxLOTWindow.h"
#include "NxDesignWindow.h"
#include "NxLoginDialog.h"
#include "NxIVProgramWindow.h"
#include "NxLogWindow.h"

namespace Ui
{
    class MainWindow;
}

class NxMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NxMainWindow( QWidget* parent = 0 );
    ~NxMainWindow();
private slots:
    void ErrorReported( NxException ex );

private:
    Ui::MainWindow* ui;
    QLabel* m_pLblVersion = nullptr;
    int m_currentTabIndex = 0;

    NxMarkWindow* m_pMarkWindow = nullptr;
    NxTransferWindow* m_pTransferWindow = nullptr;
    NxLOTWindow* m_pLOTWindow = nullptr;
    NxModelWindow* m_pModelWindow = nullptr;
    NxDesignWindow* m_pDesignWindow = nullptr;
    NxIVProgramWindow* m_pIVProgramWindow = nullptr;
    NxSettingsWindow* m_pSettingsWindow = nullptr;
    NxControlWindow* m_pControlWindow = nullptr;
    NxLogWindow* m_pLogWindow = nullptr;


    void resizeEvent( QResizeEvent* event );
    void closeEvent( QCloseEvent* );
    bool eventFilter( QObject* watched, QEvent* event );
    void OnLoginOrLogout();
    void OnLockUI();
    void OnUnLockUIForLeader();
    void OnUnLockUIForSuper();
    void OnUnLockUIForAdmin();
    void OnTabChanged( int index );
};

