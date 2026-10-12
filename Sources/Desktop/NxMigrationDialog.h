#pragma once
#include "QDialog"
#include "XDialog.h"
#include "QWidget"

#include "NxSettings.h"

namespace Ui
{
    class MigrationDialog;
}


class NxMigrationDialog : public XDialog
{
    Q_OBJECT
public:
    NxMigrationDialog( QWidget* parent = nullptr );
    ~NxMigrationDialog();

private:
    Ui::MigrationDialog* ui;
    HxRegistrySetting m_settings;

    void OnBrowseOldDataDir();
    void OnMigrationLOTs();
    void OnMigrationModels();
    void OnMigrationDesigns();
    void OnMigrationPrintLogs();
    void OnMigrationRePrintLogs();
    void OnMigrationBarcodes();
    void OnMigrationStoppers();
};