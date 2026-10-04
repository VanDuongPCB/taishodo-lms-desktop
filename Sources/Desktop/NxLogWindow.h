#pragma once
#include "QMainWindow"
#include "QLabel"
#include "QDateEdit"
#include "QLineEdit"
#include "NxLogger.h"

namespace Ui
{
    class LogWindow;
}

class NxLogWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NxLogWindow( QWidget* parent = nullptr );
    ~NxLogWindow();

private:
    Ui::LogWindow* ui;
    QDateEdit* m_pDateFrom = nullptr;
    QDateEdit* m_pDateTo = nullptr;
    QLineEdit* m_pSerial = nullptr;
    HxLogArray m_logData;


    void OnSearch();
    void OnExport();
};

