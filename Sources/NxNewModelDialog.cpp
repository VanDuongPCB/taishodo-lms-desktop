#include "NxNewModelDialog.h"
#include "ui_nxnewmodeldialog.h"
#include "NxModel.h"
#include "NxMessage.h"
#include "NxIVProgram.h"

NxNewModelDialog::NxNewModelDialog( NxModelPtrMap models, NxModelPtrMap modelToSaves, QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::NewModelDialog )
{
    ui->setupUi( this );
    m_models = models;
    m_modelToSaves = modelToSaves;

    ui->cbxIV->clear();
    ui->cbxIV->addItem( "" );
    auto programs = IVProgram()->Get();
    for ( auto& program : programs )
        ui->cbxIV->addItem( program.Name );
    connect( ui->btnCreate, &QPushButton::clicked, this, &NxNewModelDialog::OnCreate );
}

NxNewModelDialog::~NxNewModelDialog()
{
    delete ui;
}

NxModelPtr NxNewModelDialog::GetModel() const
{
    return m_pModel;
}

bool NxNewModelDialog::checkInputs()
{
    QString newName = ui->txtModelName->text().trimmed().toUpper();
    QString newCode = ui->txtModelCode->text().trimmed().toUpper();

    if ( newName.length() < 1 || newCode.length() < 1 )
    {
        HxMsgWarning( tr( "Tên model và mã model không được để trống!" ), tr( "Dữ liệu không hợp lệ" ) );
        return false;
    }
    for ( auto& [name, pModel] : m_models )
    {
        if ( pModel->Name() == newName )
        {
            HxMsgError( tr( "Model có tên \"%1\" đã tồn tại!" ).arg( newName ), tr( "Trùng tên model" ) );
            return false;
        }

        if ( pModel->Code() == newCode )
        {
            HxMsgError( tr( "Model code \"%1\" đã tồn tại!" ).arg( newCode ), tr( "Trùng tên model code" ) );
            return false;
        }
    }

    for ( auto& [name, pModel] : m_modelToSaves )
    {
        if ( pModel->Name() == newName )
        {
            HxMsgError( tr( "Model có tên \"%1\" đã tồn tại!" ).arg( newName ), tr( "Trùng tên model" ) );
            return false;
        }

        if ( pModel->Code() == newCode )
        {
            HxMsgError( tr( "Model code \"%1\" đã tồn tại!" ).arg( newCode ), tr( "Trùng tên model code" ) );
            return false;
        }
    }
    return true;
}

void NxNewModelDialog::OnCreate()
{
    if ( checkInputs() )
    {
        m_pModel = ModelManager()->Create( ui->txtModelCode->text().trimmed(), ui->txtModelName->text().replace("*", "").trimmed());
        m_pModel->SetkNo( ui->txtModelNo->text().trimmed().toUpper() );
        m_pModel->SetCvWidth( ui->spxCvWidth->value() );
        m_pModel->SetStopper( ui->cbxStopper->currentIndex() + 1 );
        m_pModel->SetDesign( ui->spxDesign->value() );
        m_pModel->SetIVProgram( ui->cbxIV->currentText() );
        m_pModel->SetPrintLo( ui->chxPrintLo->isChecked() );
        close();
        setResult( 1 );
    }
}

