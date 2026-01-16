#pragma once
#include "QString"

class HxLicensing
{
public:
    QString ReadKey() const;
    void WriteKey( const QString& key ) const;
    bool IsRegistered();
    QString ID();
    QString KeyFromId( const QString& id ) const;
    bool RegisterKey( const QString& key );
    QString GetVersion();

private:
    QString m_Version = "2025.SP0";
    double m_Kernel = 0.202500;
};

HxLicensing* Licensing();
