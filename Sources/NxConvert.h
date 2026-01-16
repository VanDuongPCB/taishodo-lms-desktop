#pragma once
#include "stdint.h"
#include "QString"
#include "QColor"
#include "NxLOT.h"


uint64_t Uint64FromHexString( const QString& hexStr );
QString HexStringFromUint64( uint64_t value, int length );

QString ProductStatusToString(NxLOT::ProductStatus status);
QColor ProductStatusToColor(NxLOT::ProductStatus status);
