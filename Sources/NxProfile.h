#pragma once
#include <vector>
#include <memory>
#include <set>
#include "map"

#include "QString"

#include "NxObject.h"
#include "NxSettings.h"

class NxProfile;
using NxProfilePtr = std::shared_ptr<NxProfile>;
using NxProfilePtrArray = std::vector<NxProfilePtr>;
using NxProfilePtrMap = std::map<QString, NxProfilePtr>;

class NxProfile : public NxObject
{
public:
    enum ModifyType
    {
        eNew = 0x01,
        eName = 0x02,
        ePassword = 0x04,
        ePermission = 0x08
    };

    NxProfile();
    ~NxProfile();

    QString ID() const;
    QString Name() const;
    QString Password() const;
    QString Permissions() const;
    bool Permission( const QString& permission ) const;

    void SetID( const QString& ID );
    void SetName( const QString& name );
    void SetPassword( const QString& password );
    void SetPermission( const QString& permission, bool isOn );

private:
    QString m_id;
    QString m_name;
    QString m_password;
    std::set<QString> m_permissions;

public:
    static NxProfilePtr Create();
};