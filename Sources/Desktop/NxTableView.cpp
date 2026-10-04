#include "NxTableView.h"
#include "QKeyEvent"
#include "iostream"

NxTableView::NxTableView( QWidget* parent ) : QTableView( parent )
{

}

NxTableView::~NxTableView()
{

}

void NxTableView::keyPressEvent( QKeyEvent* event )
{
    QTableView::keyPressEvent( event );
    int key = event->key();
    // up + down
    if ( key == 16777235 || key == 16777237 )
    {
        emit pressed( this->currentIndex() );
    }
}

NxDataTable* NxTableView::dataTable()
{
    NxDataTable* m = ( NxDataTable* )this->model();
    if ( m == nullptr )
    {
        m = new NxDataTable();
        this->setModel( m );
    }
    return m;
}

void NxTableView::setHeaders( QStringList headers )
{
    this->headers = headers;
    NxDataTable* m = dataTable();
    m->setColumnCount( headers.size() );
    m->setHorizontalHeaderLabels( headers );
}

void NxTableView::setRowCount( int count )
{
    dataTable()->setRowCount( count );
}

int NxTableView::RowCount()
{
    return dataTable()->rowCount();
}

QStandardItem* NxTableView::item( int row, int col )
{
    return ( ( QStandardItemModel* )model() )->item( row, col );
}

QStandardItem* NxTableView::item( int row, QString header )
{
    for ( int i = 0; i < headers.size(); i++ )
    {
        if ( header == headers[ i ] )
        {
            return ( ( QStandardItemModel* )model() )->item( row, i );
        }
    }
    return nullptr;
}

void NxTableView::setText( int row, int col, QString data )
{
    NxDataTable* m = dataTable();
    if ( row < 0 || row >= m->rowCount() ) return;
    if ( col < 0 || col >= m->columnCount() ) return;
    QStandardItem* item = ( QStandardItem* )m->item( row, col );
    if ( item == nullptr )
    {
        item = new QStandardItem();
        m->setItem( row, col, item );
    }
    item->setText( data );
}

void NxTableView::setText( int row, QString header, QString data )
{
    NxDataTable* m = dataTable();
    if ( row < 0 || row >= m->rowCount() ) return;
    for ( int col = 0; col < headers.size(); col++ )
    {
        if ( headers[ col ] == header )
        {
            setText( row, col, data );
            break;
        }
    }
}


