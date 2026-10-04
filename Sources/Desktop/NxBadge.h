#pragma once
#include "stdint.h"
#include "QString"
#include "QLabel"
#include "QColor"
#include "NxLOT.h"


class NxBadge : public QLabel
{
    Q_OBJECT
public:
    NxBadge( QWidget* parent = nullptr );
    void SetValue( int value );
};