#pragma once
#include "QTableView"
#include "NxDataTable.h"

class NxTableView : public QTableView
{
public:
    NxTableView( QWidget* parent = nullptr );
    ~NxTableView();
    QStringList headers;
    NxDataTable* dataTable();
    void setHeaders( QStringList headers );
    void setRowCount( int count );
    int RowCount();
    QStandardItem* item( int row, int col );
    QStandardItem* item( int row, QString header );
    void setText( int row, int col, QString data );
    void setText( int row, QString header, QString data );
protected:
    void keyPressEvent( QKeyEvent* event );
};

