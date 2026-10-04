#include "NxBadge.h"

NxBadge::NxBadge( QWidget* parent ) : QLabel( parent )
{
    setText( "0" );
    QFont _font = font();
    _font.setPointSizeF( 8 );
    setFont( _font );
    setMinimumWidth( 14 );
    setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    setStyleSheet( "background-color:red; color:#fff; padding:0px; border-radius:3px;" );
    setVisible( false );
}

void NxBadge::SetValue( int value )
{
    setText( QString::number( value ) );
    setVisible( value != 0 );
}