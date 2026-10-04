#pragma once
#include "memory"
#include "vector"

#include "QMap"
#include "QString"
#include "QObject"

#include "NxDefines.h"
#include "NxBlock.h"
#include "NxObject.h"
#include "NxSettings.h"

class NxDesign;
using NxDesignPtr = std::shared_ptr< NxDesign >;
using NxDesignPtrMap = std::map<QString, NxDesignPtr >;

class NxDesign : public NxObject
{
public:
    enum ModifyType
    {
        eNew = 0x01,
        eSize = 0x02,
        eBlock = 0x04
    };

    NxDesign();
    ~NxDesign();

    QString Name() const;
    double Width() const;
    double Height() const;
    NxBlock Block( int index );
    std::map<int, NxBlock> Blocks();
    int IndexOfBlockCode();

    void SetName( const QString& name );
    void SetWidth( double value );
    void SetHeight( double value );
    void SetBlock( int index, const NxBlock& block );
    
private:
    QString m_name = "";
    double m_width = 10;
    double m_height = 10;
    std::map<int, NxBlock> m_blocks;
};

class NxDesignManager : private QObject
{
    Q_OBJECT
public:
    NxDesignManager();
    NxDesignPtr Create();
    NxDesignPtr GetDesign( const QString& name );
    NxDesignPtrMap GetDesigns();
    ReturnCode Save( NxDesignPtr pDesign, bool bForce = false );
    ReturnCode DeleteAll();
    void ReLoadSetting();
private:
    HxRegistrySetting m_settings;
};

NxDesignManager* DesignManager();