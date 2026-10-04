#pragma once
#include "QMainWindow"
#include "NxSettings.h"
#include "NxProfile.h"
#include "NxStopper.h"

namespace Ui
{
    class SettingsWindow;
}

class NxSettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NxSettingsWindow( QWidget* parent = 0 );
    ~NxSettingsWindow();

private:
    Ui::SettingsWindow* ui;
    HxRegistrySetting m_regSettings;
    NxStopperPtrMap m_pStoppers;

    void showEvent( QShowEvent* );
    bool eventFilter( QObject* watched, QEvent* event );
    void Show();

    void OnLoad();
    void OnBrowseDataDir();
    void OnBrowseIVDir();
    void OnEnumerateLaserPorts();
    void OnSave();
    void OnMigration();
};

