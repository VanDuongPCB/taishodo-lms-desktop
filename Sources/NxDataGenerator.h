#pragma once

#include "NxDesign.h"
#include "NxModel.h"
#include "NxLOT.h"

QString GenMarkData( const QString& format, NxLOTPtr pLOT, NxModelPtr pModel, int shift = 0 );
std::map<int, QString> GenMarkData( NxDesignPtr pDesign, NxLOTPtr pLOT, NxModelPtr pModel, int shift = 0 );