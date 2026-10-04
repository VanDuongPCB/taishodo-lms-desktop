#pragma once
#include "QObject"

class NxObject
{
public:
    NxObject();
    void SetModified( uint64_t flags );
    bool IsMofified( uint64_t flags = UINT64_MAX ) const;
    uint64_t ModifyFlags() const;
    void ClearModified();
    
private:
    uint64_t m_flags = 0;

protected:
    template<typename T>
    void Modify( T& field, const T& value, uint64_t flag = UINT64_MAX )
    {
        if ( field != value )
        {
            field = value;
            m_flags |= flag;
        }
    }
};
