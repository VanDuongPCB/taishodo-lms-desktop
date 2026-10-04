#pragma once
#include "QTableView"
#include "QStandardItemModel"
class NxDataTable : public QStandardItemModel
{
public:
    NxDataTable();
    NxDataTable( int rows, int cols );
    ~NxDataTable();


public:
    static NxDataTable* FromTableView( QTableView* table, QStringList headers );
};

