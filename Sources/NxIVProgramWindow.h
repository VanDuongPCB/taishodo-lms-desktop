#pragma once
#include "QMainWindow"

#include "NxLOT.h"
#include "NxException.h"

namespace Ui
{
    class IVWindow;
}

class NxIVProgramWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NxIVProgramWindow( QWidget* parent = nullptr );
    ~NxIVProgramWindow();

private:
    Ui::IVWindow* ui;
    void OnLoad();
};

