#pragma once
#include "QString"
#include "QMessageBox"
#include "QObject"

using HxMsgIcon = QMessageBox::Icon;
using HxMsgButton = QMessageBox::StandardButton;
using HxMsgButtons = QMessageBox::StandardButtons;

int HxMsgShow( const QString& content );
int HxMsgShow( const QString& content, const QString& title );
int HxMsgShow( const QString& content, const QString& title, HxMsgIcon icon );

int HxMsgInfo( const QString& content );
int HxMsgInfo( const QString& content, const QString& title );

int HxMsgQuestion( const QString& content );
int HxMsgQuestion( const QString& content, const QString& title );
int HxMsgQuestion( const QString& content, const QString& title, HxMsgButtons buttons );

int HxMsgWarning( const QString& content, bool bIsHighlight = false );
int HxMsgWarning( const QString& content, const QString& title, bool bIsHighlight = false );

int HxMsgError( const QString& content, bool bIsHighlight = false );
int HxMsgError( const QString& content, const QString& title, bool bIsHighlight = false );
int HxMsgError( const QString& content, const QString& details, const QString& title, bool bIsHighlight = false );
