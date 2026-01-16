#pragma once
#include "QObject"
#include "QThread"
#include "atomic"
#include "NxLOT.h"
#include "NxDesign.h"
#include "NxModel.h"
#include "NxStopper.h"
#include "NxEvent.h"



class NxMarker : public QObject
{
    Q_OBJECT
public:
    enum State
    {
        eOnUnInit,
        eOnFree,
        eOnTesting,
        eOnTransfering,
        eOnMarking,
        eOnFinish,
        eOnStopping,
        eOnError
    };

    enum SetupMode
    {
        eModeTransfer,
        eModeMarking
    };

    NxMarker();
    ~NxMarker();
    State GetState() const;
    void Init();
    void DeInit();
    bool Setup( SetupMode mode, const QString& param );
    void Mark();
    void Transfer();
    void Test();
    void Pause();
    void Stop();
    NxLOTPtr LOT() const;
    NxModelPtr Model() const;
    NxDesignPtr Design() const;
    void ReLoadSetting();
private:
    NxLOTPtr m_pLOT;
    NxModelPtr m_pModel;
    NxDesignPtr m_pDesign;
    NxStopperPtr m_pStopper;
    State m_state = eOnUnInit;

    void CheckAndPostEvent( State& lastState, State currentState, NxEvent::Type eventType );
    void Task();

private:
    HxRegistrySetting m_settings;
};


NxMarker* Marker();