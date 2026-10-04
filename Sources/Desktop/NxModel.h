#pragma once
#include "vector"
#include "vector"
#include "memory"

#include "QString"
#include "QMap"
#include "QJsonObject"

#include "NxDefines.h"
#include "NxPosition.h"
#include "NxObject.h"
#include "NxSettings.h"

class NxModel;
using NxModelPtr = std::shared_ptr<NxModel>;
using NxModelPtrArray = std::vector<NxModelPtr>;
using NxModelPtrMap = std::map<QString, NxModelPtr>;

class NxModel : public NxObject
{
public:
    enum ModifyType
    {
        eNew = 0x01,
        eInfo = 0x02,
        ePosition = 0x04,
        eComment = 0x08
    };
public:
    NxModel();
    ~NxModel();

    QString Code() const;
    QString Name( bool bModifyMask = false ) const;
    bool IsPrintLo() const;
    QString kNo() const;
    QString IVProgram() const;
    QString Design() const;
    double CvWidth() const;
    int Stopper() const;
    std::map<int, NxPosition>& Positions();
    NxPosition Position( int index ) const;
    std::map<QString, QString>& Comments();
    QString Comment( const QString& name ) const;
    QString Value( QString paramName ) const;

    void SetCode( const QString& value );
    void SetName( const QString& value );
    void SetPrintLo( bool bIsEnable );
    void SetkNo( const QString& value );
    void SetIVProgram( const QString& value );
    void SetDesign( const QString& value );
    void SetDesign( size_t value );
    void SetCvWidth( double value );
    void SetStopper( int value );
    void SetPositions( const std::map<int, NxPosition>& value );
    void SetPosition( int index, const NxPosition& value );
    void RemovePosition( int index );
    void SetComments( const std::map<QString, QString>& comments );
    void SetComment( const QString& key, const QString& value );
    void AddComments( const QStringList& keys);
    void RemoveComments( const QStringList& keys );
    void SetValue( const QString& key, const QString& value );

private:
    QString m_code;
    QString m_name;
    bool m_bIsPrintLo = false;
    QString m_kNo;
    QString m_ivProgram;
    QString m_design;
    double m_cvWidth = 0;
    int m_stopper = 1;
    std::map<int, NxPosition> m_positions;
    std::map<QString, QString> m_comments;

public:
    static QStringList paramNames();
};


class NxModelManager : private QObject
{
    Q_OBJECT
public:
    NxModelManager();
    NxModelPtr Create( const QString& code = QString(), const QString& name = QString() );
    QStringList Names();
    QStringList ParamNames();
    NxModelPtrMap GetModels();
    NxModelPtr GetModel( const QString& code );
    ReturnCode Save( NxModelPtr pModel );
    ReturnCode Delete( const QString& code );
    void AddComments( const QStringList& keys );
    void RemoveComments( const QStringList& keys );
    ReturnCode DeleteAll();
    void ReloadSetting();

private:
    HxRegistrySetting m_settings;
};

NxModelManager* ModelManager();
