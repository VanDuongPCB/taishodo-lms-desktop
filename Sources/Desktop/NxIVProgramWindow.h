#pragma once
#include "QMainWindow"

#include "XMainWindow.h"
#include "NxLOT.h"
#include "NxException.h"

namespace Ui
{
    class IVWindow;
}

class NxIVProgramWindow : public XMainWindow
{
    Q_OBJECT

public:
    explicit NxIVProgramWindow( QWidget* parent = nullptr );
    ~NxIVProgramWindow();

private:
    Ui::IVWindow* ui;
    void OnLoad();
};

