#include "NxDataTable.h"

NxDataTable::NxDataTable() : QStandardItemModel( nullptr )
{

}

NxDataTable::NxDataTable( int rows, int cols ) : QStandardItemModel( rows, cols, nullptr )
{

}

NxDataTable::~NxDataTable()
{

}

