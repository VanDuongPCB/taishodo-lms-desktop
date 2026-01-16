#pragma once
#include "QDialog"

#include "NxLOT.h"
#include "NxModel.h"

namespace Ui
{
	class LOTSelector;
}

class NxLOTSelector : public QDialog
{
public:
	NxLOTSelector(QWidget* parent  = nullptr);
	~NxLOTSelector();

private:
	Ui::LOTSelector* ui;
	NxLOTPtrMap m_LOTs;
	NxModelPtrMap m_Models;

	NxModelPtr FindModel( const QString& name );
	void OnShow();
	void OnSelected( const QModelIndex& index );
};
