#include "NxLOTPropertyDialog.h"
#include "ui_nxlotpropertydialog.h"
#include "NxLOT.h"
#include "NxModel.h"
#include "NxDesign.h"
#include "NxConvert.h"
#include "NxMessage.h"
#include "NxDataGenerator.h"

NxLOTPropertyDialog::NxLOTPropertyDialog( NxLOTPtr pLOT, NxLOTPtrMap LOTs, QWidget* parent ) : QDialog( parent ), ui( new Ui::LotPropertyDialog )
{
    ui->setupUi( this );
    m_LOTs = LOTs;
    ui->tbvParams->setHeaders( { "Thông tin","Giá trị" } );
    ui->tbvBlocks->setHeaders( { "Block","Nội dung","Độ dài","Định dạng" } );
    ui->tbvBlocks->setColumnWidth( 1, 300 );

    OnInit( pLOT );
    ShowInfo();
    ShowPrintLo();
    ShowParams();
    ShowBlocks();

    connect( ui->txtName, &QLineEdit::textChanged, this, &NxLOTPropertyDialog::OnInfoChanged );
    connect( ui->txtCounterStart, &QLineEdit::textChanged, this, &NxLOTPropertyDialog::OnInfoChanged );
    connect( ui->txtMACStart, &QLineEdit::textChanged, this, &NxLOTPropertyDialog::OnInfoChanged );
    connect( ui->txtMACEnd, &QLineEdit::textChanged, this, &NxLOTPropertyDialog::OnInfoChanged );

    connect( ui->cbxModel, &QComboBox::currentTextChanged, this, &NxLOTPropertyDialog::OnModelChanged );
    connect( ui->btnCreateOrChange, &QPushButton::clicked, this, &NxLOTPropertyDialog::OnApply );
}

NxLOTPropertyDialog::~NxLOTPropertyDialog()
{
    delete ui;
}

NxLOTPtr NxLOTPropertyDialog::GetLOT() const
{
    return m_pLOT;
}

void NxLOTPropertyDialog::OnInit( NxLOTPtr pLOT )
{
    QSignalBlocker blocker( this );
    m_pModel.reset();
    m_pDesign.reset();
    m_pLOT.reset();
    if(pLOT )
        m_pLOT = pLOT->Clone();
    if ( m_pLOT )
        m_pModel = ModelManager()->GetModel( m_pLOT->Model() );
    if ( m_pModel )
        m_pDesign = DesignManager()->GetDesign( m_pModel->Design() );

    m_bIsCreate = m_pLOT == nullptr;
    if ( m_bIsCreate )
    {
        m_pLOT = LOTManager()->Create();
        setWindowTitle( "Tạo LOT mới" );
        ui->btnCreateOrChange->setText( "Tạo LOT mới" );
        ui->txtName->setReadOnly( false );
    }
    else
    {
        setWindowTitle( "Thay đổi thông tin LOT" );
        ui->btnCreateOrChange->setText( "Cập nhật LOT" );
        ui->txtName->setReadOnly( true );
    }

    ui->cbxModel->addItem( "" );
    ui->cbxModel->addItems( ModelManager()->Names() );

    switch ( m_pLOT->Status() )
    {
    case NxLOT::ePending:
        ui->cbxModel->setEnabled( true );
        ui->txtMACStart->setEnabled( true );
        ui->txtCounter->setEnabled( true );
        ui->txtMACEnd->setEnabled( true );
        ui->spxQuantity->setEnabled( true );
        ui->txtCounterStart->setEnabled( true );
        break;
    case NxLOT::eProduct:
        ui->cbxModel->setEnabled( false );
        ui->txtMACStart->setEnabled( false );
        ui->txtCounter->setEnabled( true );
        ui->txtMACEnd->setEnabled( true );
        ui->spxQuantity->setEnabled( true );
        ui->txtCounterStart->setEnabled( false );
        break;

    case NxLOT::eCompleted:
        ui->cbxModel->setEnabled( false );
        ui->txtMACStart->setEnabled( false );
        ui->txtCounter->setEnabled( true );
        ui->txtMACEnd->setEnabled( true );
        ui->spxQuantity->setEnabled( true );
        ui->txtCounterStart->setEnabled( false );
        break;
    default:
        break;
    }
}

void NxLOTPropertyDialog::ShowInfo()
{
    QSignalBlocker blocker( this );
    ui->txtName->setText( m_pLOT->Name() );
    ui->cbxModel->setCurrentText( m_pLOT->Model() );
    ui->txtCounter->setText( m_pLOT->Counter() );
    ui->txtMACStart->setText( m_pLOT->MACStart() );
    ui->txtMACEnd->setText( m_pLOT->MACEnd() );
    ui->spxQuantity->setValue( m_pLOT->Quantity() );
    ui->txtCounterStart->setText( m_pLOT->CounterStart() );
    if ( m_pModel )
    {
        ui->txtDesign->setText( m_pModel->Design() );
        ui->txtProgram->setText( m_pModel->IVProgram() );
        ui->chxRePrint->setVisible( m_pModel->IsPrintLo() );
    }
    else
    {
        ui->txtDesign->setText( "" );
        ui->txtProgram->setText( "" );
        ui->chxRePrint->setVisible( false );
    }
}

void NxLOTPropertyDialog::ShowPrintLo()
{
    ui->chxRePrint->setVisible( false );
    m_pDesign.reset();
    m_pModel = ModelManager()->GetModel( m_pLOT->Model() );
    if ( m_pModel == nullptr || m_pModel->IsPrintLo() == false )
        return;
    ui->chxRePrint->setVisible( true );
}

void NxLOTPropertyDialog::ShowParams()
{
    QSignalBlocker blocker( this );
    disconnect( ui->tbvParams->dataTable(), &QStandardItemModel::itemChanged, this, &NxLOTPropertyDialog::OnParamChanged );

    QStringList paramNames = LOTManager()->Parameters();
    auto comments = m_pLOT->Comments();
    for ( auto& [key, value] : comments )
    {
        if ( !paramNames.contains( key ) )
            paramNames.push_back( key );
    }

    int rows = paramNames.size();
    ui->tbvParams->setRowCount( rows );

    int row = 0;
    for ( auto& name : paramNames )
    {
        ui->tbvParams->setText( row, 0, name );
        auto it = comments.find( name );
        if ( it != comments.end() )
            ui->tbvParams->setText( row, 1, it->second );
        else
            ui->tbvParams->setText( row, 1, "" );
        row++;
    }

    connect( ui->tbvParams->dataTable(), &QStandardItemModel::itemChanged, this, &NxLOTPropertyDialog::OnParamChanged );
}

void NxLOTPropertyDialog::ShowBlocks()
{
    QSignalBlocker blocker( this );
    GetInputs();
    ui->tbvBlocks->setRowCount( 0 );
    m_pModel = ModelManager()->GetModel( m_pLOT->Model() );
    if ( !m_pModel )
        return;

    m_pDesign = DesignManager()->GetDesign( m_pModel->Design() );
    if ( !m_pDesign )
        return;

    auto blocks = m_pDesign->Blocks();
    int maxRows = 32;
    ui->tbvBlocks->setRowCount( maxRows );
    for ( int row = 0; row < maxRows; row++ )
    {
        ui->tbvBlocks->setText( row, "Block", QString::number( row ).rightJustified( 3, '0' ) );
        bool bIsMatch = true;
        auto it = blocks.find( row );
        if ( it == blocks.end() )
        {
            ui->tbvBlocks->setText( row, "Nội dung", "");
            ui->tbvBlocks->setText( row, "Độ dài", "");
            ui->tbvBlocks->setText( row, "Định dạng", "");
        }
        else
        {
            QString data = GenMarkData( it->second.data, m_pLOT, m_pModel );
            ui->tbvBlocks->setText( row, "Nội dung", data );
            ui->tbvBlocks->setText( row, "Độ dài", QString::number( data.length() ) + "/" + QString::number( it->second.textLen ) );
            ui->tbvBlocks->setText( row, "Định dạng", it->second.data );
            bIsMatch &= ( data.length() == it->second.textLen );
        }
        ui->tbvBlocks->item( row, 0 )->setAccessibleText( bIsMatch ? "Match" : "" );
        QColor color = bIsMatch? QColor( 0, 128, 0 ) : QColor( 128, 0, 0 );
        for ( int col = 1; col < ui->tbvBlocks->dataTable()->columnCount(); col++ )
        {
            ui->tbvBlocks->item( row, col )->setForeground( color );
        }
    }
}

bool NxLOTPropertyDialog::CheckLotName()
{
    if ( !m_bIsCreate )
        return true;

    if ( m_pLOT->Name().isEmpty() )
    {
        HxMsgError( tr( "Tên LOT không được để trống!" ),
                    tr( "Lỗi" ) );
        return false;
    }

    for ( auto& [name, pLOT] : m_LOTs )
    {
        if ( pLOT->Name() == m_pLOT->Name() )
        {
            HxMsgError( tr( "Phát hiện trùng tên LOT!\n"
                            "Trường hợp bắt buộc tên LOT phải giống, hãy thêm vào phía sau một cụm từ bắt đầu bằng \"-RE\".\n"
                            "Ví dụ:\n"
                            "TÊN LOT-RE01" ),
                        tr( "Trùng lặp tên LOT" ) );
            return false;
        }
    }
    return true;
}

bool NxLOTPropertyDialog::CheckSeriRange()
{
    if ( !m_pLOT )
        return false;

    if ( ui->chxRePrint->isVisible() )
        return true;

    int numStart = m_pLOT->CounterStart().toInt();
    int numEnd = numStart + m_pLOT->Quantity() - 1;
    int minSerialLen = QString::number( numEnd ).length();
    if ( minSerialLen > m_pLOT->CounterStart().length() )
    {
        HxMsgError( tr( "Độ dài của serial bắt đầu không đủ để chứa sản lượng!" ),
                    tr( "Lỗi" ) );
        return false;
    }

    auto pModel = ModelManager()->GetModel( m_pLOT->Model() );
    if ( !pModel )
        return true;

    auto pDesign = DesignManager()->GetDesign( pModel->Design() );
    if ( !pDesign )
        return true;

    int indexBlockCode = pDesign->IndexOfBlockCode();
    if ( indexBlockCode < 1 )
        return true;
    NxBlock block = pDesign->Block( indexBlockCode );

    NxLOTPtr pFrom = m_pLOT->Clone();
    pFrom->SetProgress( 0 );
    QString startSerial = GenMarkData( block.data, pFrom, pModel );
    pFrom->SetProgress( pFrom->Quantity() - 1 );
    QString endSerial = GenMarkData( block.data, pFrom, pModel );


    for ( auto& [name, pLOT] : m_LOTs )
    {
        if ( pFrom->Name() == pLOT->Name() )
            continue;

        if ( pFrom->Model() != pLOT->Model() )
            continue;

        if ( pFrom->CounterStart().length() != pLOT->CounterStart().length() )
            continue;

        NxLOTPtr pTo = pLOT->Clone();
        pTo->SetProgress( 0 );
        QString startSerial2 = GenMarkData( block.data, pTo, pModel );
        pTo->SetProgress( pTo->Quantity() - 1 );
        QString endSerial2 = GenMarkData( block.data, pTo, pModel );

        std::vector<QString> serials{ startSerial,endSerial,startSerial2,endSerial2 };
        std::sort( serials.begin(), serials.end() );
        bool bIsInter = ( serials[ 1 ] == serials[ 2 ] ) || ( startSerial == serials[ 0 ] && endSerial != serials[ 1 ] ) || ( startSerial == serials[ 2 ] && endSerial != serials[ 3 ] );
        if ( bIsInter )
        {
            HxMsgError( tr( "PHÁT HIỆN TRÙNG TEM\n"
                            "LIÊN LẠC QUẢN LÝ NGAY !!!" ),
                        tr( "Tên LOT: %1\n"
                            "Dải tem: %2 - %3" ).arg( pLOT->Name() ).arg( startSerial2 ).arg( endSerial2 ),
                        tr( "Trùng lặp tem" ),
                        true );
            return false;
        }
    }
    return true;
}

bool NxLOTPropertyDialog::CheckMacs()
{
    if ( !m_pLOT )
        return false;

    if ( m_pDesign )
    {
        auto blocks = m_pDesign->Blocks();
        for ( auto& [index, block] : blocks )
        {
            if ( !block.data.contains( "LOT.MAC" ) )
                continue;
            if ( m_pLOT->MACStart().length() != 12 || m_pLOT->MACEnd().length() != 12 )
            {
                HxMsgError( tr( "Thiết kế tem yêu cầu thông tin MAC nhưng cài đặt chưa hợp lệ!" ), tr( "Thiếu thông tin MAC" ) );
                return false;
            }
        }
    }

    if ( m_pLOT->MACStart().isEmpty() && m_pLOT->MACEnd().isEmpty() )
        return true;

    if ( m_pLOT->MACStart().length() != 12 )
    {
        HxMsgError( tr( "MAC bắt đầu không hợp lệ!" ), tr( "MAC không hợp lệ" ) );
        return false;
    }

    if ( m_pLOT->MACEnd().length() != 12 )
    {
        HxMsgError( tr( "MAC kết thúc không hợp lệ!" ), tr( "MAC không hợp lệ" ) );
        return false;
    }

    std::string smac = ( m_pLOT->MACStart() + m_pLOT->MACEnd() ).toStdString();
    for ( int i = 0; i < smac.length(); i++ )
    {
        char c = smac[ i ];
        if ( ( c >= '0' && c <= '9' ) || ( c >= 'A' && c <= 'F' ) ) continue;
        HxMsgError( tr( "MAC chứa ký tự không hợp lệ!" ), tr( "MAC không hợp lệ" ) );
        return false;
    }

    uint64_t numMACStart = Uint64FromHexString( m_pLOT->MACStart() );
    uint64_t numMACEnd = Uint64FromHexString( m_pLOT->MACEnd() );
    if ( numMACStart > numMACEnd )
    {
        HxMsgError( tr( "MAC bắt đầu không được lớn hơn MAC kết thúc!" ), tr( "MAC không hợp lệ" ) );
        return false;
    }

    int totalMAC = numMACEnd - numMACStart + 1;
    if ( totalMAC != m_pLOT->Quantity() )
    {
        HxMsgError( tr( "PHÁT HIỆN SAI DẢI MAC\n"
                        "LIÊN LẠC QUẢN LÝ NGAY !!!" ),
                    tr( "Tên LOT: %1\n"
                        "Dải MAC: %2 - %3, số lượng: %4\n"
                        "Sản lượng cài đặt: %5" )
                    .arg( m_pLOT->Name() )
                    .arg( m_pLOT->MACStart() )
                    .arg( m_pLOT->MACEnd() )
                    .arg( totalMAC )
                    .arg( ( m_pLOT->Quantity() ) ),
                    tr( "Sai dải MAC" ),
                    true );
        return false;
    }

    if ( ui->chxRePrint->isVisible() )
        return true;

    for ( auto& [name, pLOT] : m_LOTs )
    {
        if ( pLOT->Name() == m_pLOT->Name() )
            continue;

        if ( pLOT->MACStart().length() + pLOT->MACEnd().length() != 24 )
            continue;

        uint64_t checkMACStart = Uint64FromHexString( pLOT->MACStart() );
        uint64_t checkMACEnd = Uint64FromHexString( pLOT->MACEnd() );
        if ( checkMACStart > checkMACEnd )
            continue;

        std::vector<uint64_t> macs = { numMACStart, numMACEnd, checkMACStart, checkMACEnd };
        std::sort( macs.begin(), macs.end() );

        bool bIsInter = ( macs[ 1 ] == macs[ 2 ] ) || ( numMACStart == macs[ 0 ] && numMACEnd != macs[ 1 ] ) || ( numMACStart == macs[ 2 ] && numMACEnd != macs[ 3 ] );

        if ( bIsInter )
        {
            HxMsgError( tr( "PHÁT HIỆN TRÙNG MAC\n"
                            "LIÊN LẠC QUẢN LÝ NGAY !!!" ),
                        tr( "Tên LOT: %1\n"
                            "Dải MAC: %2 - %3" ).arg( pLOT->Name() ).arg( pLOT->MACStart() ).arg( pLOT->MACEnd() ),
                        tr( "Trùng MAC" ),
                        true );
            return false;
        }
    }
    return true;
}

bool NxLOTPropertyDialog::CheckModelInfo()
{
    auto pModel = ModelManager()->GetModel( m_pLOT->Model() );
    if ( !pModel )
    {
        HxMsgError( tr( "Không tồn tại model: %1!" ).arg( m_pLOT->Model() ),
                    tr( "Thông tin cài đặt không đúng" ) );
        return false;
    }

    auto pDesign = DesignManager()->GetDesign( pModel->Design() );
    if ( !pDesign )
    {
        HxMsgError( tr( "Không tồn tại mẫu tem: %1!" ).arg( pModel->Design() ),
                    tr( "Tên mẫu tem: %1" ).arg( pModel->Design() ),
                    tr( "Thông tin cài đặt không đúng" ) );
        return false;
    }

    return true;
}

bool NxLOTPropertyDialog::CheckBlocks()
{
    ShowBlocks();
    int rows = ui->tbvBlocks->dataTable()->rowCount();
    for ( int row = 0; row < rows; row++ )
    {
        QString text = ui->tbvBlocks->item( row, 0 )->accessibleText();
        if ( text.length() == 0 )
        {
            HxMsgError( tr( "Phát hiện một số block có dữ liệu không đúng!" ),
                        tr( "Thông tin block không đúng" ) );
            return false;
        }
    }
    return true;
}

void NxLOTPropertyDialog::GetInputs()
{
    m_pLOT->SetName( ui->txtName->text().trimmed().toUpper() );

    m_pLOT->SetCounterStart( ui->txtCounterStart->text().trimmed() );
    m_pLOT->SetQuantity( ui->spxQuantity->value() );

    m_pLOT->SetMACStart( ui->txtMACStart->text().trimmed().toUpper() );
    m_pLOT->SetMACEnd( ui->txtMACEnd->text().trimmed().toUpper() );

    m_pLOT->SetModel( ui->cbxModel->currentText() );

}

bool NxLOTPropertyDialog::CheckInputs()
{
    GetInputs();
    return CheckLotName() && CheckModelInfo() && CheckSeriRange() && CheckMacs() && CheckBlocks();
}

void NxLOTPropertyDialog::OnInfoChanged()
{
    ShowBlocks();
}

void NxLOTPropertyDialog::OnParamChanged( QStandardItem* item )
{
    int row = item->row();
    QString key = ui->tbvParams->item( row, 0 )->text();
    QString value = ui->tbvParams->item( row, 1 )->text().trimmed().toUpper();
    m_pLOT->SetValue( key, value );
    ShowBlocks();
    CheckInputs();
}

void NxLOTPropertyDialog::OnModelChanged()
{
    m_pLOT->SetModel( ui->cbxModel->currentText() );
    m_pModel = ModelManager()->GetModel( m_pLOT->Model() );
    m_pDesign.reset();
    if ( m_pModel )
    {
        m_pDesign = DesignManager()->GetDesign( m_pModel->Design() );
        ui->txtDesign->setText( m_pModel->Design() );
        ui->txtProgram->setText( m_pModel->IVProgram() );
        ui->chxRePrint->setVisible( m_pModel->IsPrintLo() );
    }
    else
    {
        ui->txtDesign->setText( "-" );
        ui->txtProgram->setText( "-" );
        ui->chxRePrint->setVisible( false );
    }
    ShowBlocks();
}

void NxLOTPropertyDialog::OnApply()
{
    if ( !CheckInputs() )
        return;

    //LOTManager()->Save( m_pLOT );
    close();
    setResult( 1 );
}
