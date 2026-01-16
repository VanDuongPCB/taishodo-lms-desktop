#pragma once
#include "QString"
#include "QStringList"
#include "QMap"
#include "vector"
#include "memory"

#include "NxDefines.h"
#include "NxSettings.h"


struct NxStopper;
using NxStopperPtr = std::shared_ptr<NxStopper>;
using NxStopperPtrMap = std::map<int, NxStopperPtr>;

struct NxStopper
{
    double x = -150.0;
    double y = -127.0;
};

class NxStopperManager : private QObject
{
    Q_OBJECT
public:
    NxStopperManager();
    NxStopperPtr Create();
    NxStopperPtr GetStopper( int index );
    NxStopperPtrMap GetStoppers();
    ReturnCode Save( int index, NxStopperPtr pStopper );
    void ReLoadSetting();
private:
    std::map<int, NxStopperPtr> m_items;
    HxRegistrySetting m_settings;
};

NxStopperManager* StopperManager();