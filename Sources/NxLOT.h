#pragma once
#include "vector"
#include "memory"

#include "QString"
#include "QMap"
#include "QDate"

#include "NxDefines.h"
#include "NxObject.h"
#include "NxSettings.h"

class NxLOT;
using NxLOTPtr = std::shared_ptr<NxLOT>;
using NxLOTPtrArray = std::vector<NxLOTPtr>;
using NxLOTPtrMap = std::map<QString, NxLOTPtr>;

class NxLOT : public NxObject
{
public:
    enum ProductStatus
    {
        eProduct = 0,
        ePending = 1,
        eCompleted = 2
    };

    enum ModifyFlag : uint64_t
    {
        eNew = 0x01,
        eInfo = 0x02,
        eProgress = 0x04,
        eComment = 0x08
    };

    NxLOT();
    ~NxLOT();

    QString Name( bool bModifyMask = false ) const;
    QString CounterStart() const;
    QString CounterEnd() const;
    QString Counter( int shift = 0 ) const;
    QString MACStart() const;
    QString MACEnd() const;
    QString MAC( int shift = 0 ) const;
    int Quantity() const;
    int Progress() const;
    QString Model() const;
    bool IsRePrint() const;
    QString Value( QString paramName ) const;
    std::map<QString, QString> Comments() const;
    bool IsCompleted();
    ProductStatus Status() const;
    void Evaluate();

    void SetName( const QString& value );
    void SetCounterStart( const QString& value );
    void SetMACStart( const QString& value );
    void SetMACEnd( const QString& value );
    void SetQuantity( int value );
    void SetProgress( int value );
    void SetModel( const QString& value );
    void SetRePrint( bool bIsEnabled );
    void SetValue( const QString& name, const QString& value );

    bool NextItem();
    NxLOTPtr Clone() const;

private:
    QString m_name;
    QString m_macStart = "";
    QString m_macEnd = "";
    QString m_counterStart = "";
    int m_quantity = 1;
    int m_progress = 0;
    bool m_isRePrint = false;
    QString m_modelName;
    std::map<QString, QString> m_comments;
    ProductStatus m_status = ePending;
};

class NxLOTManager: public QObject
{
    Q_OBJECT
public:
    explicit NxLOTManager();
    NxLOTPtr Create();
    NxLOTPtr GetLOT( const QString& lotName );
    NxLOTPtrMap GetLOTs( const QDate& fromTime = QDate( 2000, 1, 1 ) );
    ReturnCode Save( NxLOTPtr pLOT );
    ReturnCode Delete( NxLOTPtr pLOT );
    ReturnCode Delete( const QString& name );
    ReturnCode Deletes( const std::set<QString>& names );
    QStringList Parameters();
    ReturnCode DeleteAll();
    void ReloadSetting();

private:
    HxRegistrySetting m_settings;
    QStringList m_paramNames;
};

NxLOTManager* LOTManager();
