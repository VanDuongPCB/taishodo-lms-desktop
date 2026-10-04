#pragma once
#include "QMainWindow"

namespace Ui
{
    class ControlWindow;
}

class NxControlWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NxControlWindow( QWidget* parent = 0 );
    ~NxControlWindow();

private slots:
    void on_btnBarcodeRead_clicked();

    void on_spxCvWidth_valueChanged( double arg1 );

    void on_cbxStopper_currentIndexChanged( int index );

    void on_btnBlockClear_clicked();

    void on_btnMark_clicked();

private:
    Ui::ControlWindow* ui;
};
