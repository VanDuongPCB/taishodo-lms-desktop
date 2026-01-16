#include "NxMarker.h"
#include "NxStopper.h"
#include <QTimer>
#include <QSerialPort>
#include <QTcpSocket>

#include "NxLogger.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "QtConcurrent"



#include "NxMessage.h"


#include "NxLaser.h"
#include "NxPLC.h"
#include "NxStopper.h"
#include "NxBarcode.h"

#include "NxException.h"
#include "NxSystemError.h"

#include "NxDataGenerator.h"
#include "NxEvent.h"

#include "NxDefines.h"


NxMarker::NxMarker() : QObject( nullptr )
{
    m_settings.Load();
}

NxMarker::~NxMarker()
{

}

NxMarker::State NxMarker::GetState() const
{
    return m_state;
}

void NxMarker::Init()
{
    if ( m_state != eOnUnInit )
        return;

    QtConcurrent::run( &NxMarker::Task, this );
}

void NxMarker::DeInit()
{
    m_state = eOnStopping;
}

bool NxMarker::Setup( SetupMode mode, const QString& param )
{
    if ( m_state != eOnFree && m_state != eOnFinish )
    {
        HxMsgError( tr( "Đang khắc hoặc đang vận chuyển!" ), tr( "Cài đặt không thành công" ) );
        return false;
    }

    m_pLOT.reset();
    m_pModel.reset();
    m_pDesign.reset();
    m_pStopper.reset();

    if ( mode == eModeTransfer )
    {
        NxModelPtr pModel = ModelManager()->GetModel( param );
        if ( !pModel )
        {
            HxMsgError( tr( "Không tìm thấy dữ liệu model: %1!" ).arg( param ), tr( "Cài đặt không thành công" ) );
            return false;
        }

        try
        {
            PLC()->SetCvWidth( pModel->CvWidth() );
            PLC()->SetStopper( pModel->Stopper() );
        }
        catch ( NxException ex )
        {
            qApp->postEvent( qApp, new NxEvent( NxEvent::eMarkerGoError, ex.toJsonObject() ) );
            HxMsgError( ex.Message(), ex.Where() );
            return false;
        }

        m_pModel = pModel;
        qApp->postEvent( qApp, new NxEvent( NxEvent::eMarkerSetupChanged ) );
        return true;
    }

    if ( mode == eModeMarking )
    {
        NxLOTPtr pLOT = LOTManager()->GetLOT( param );
        if ( !pLOT )
        {
            HxMsgError( tr( "Không tìm thấy dữ liệu LOT: %1!" ).arg( param ), tr( "Cài đặt không thành công" ) );
            return false;
        }

        NxModelPtr pModel = ModelManager()->GetModel( pLOT->Model() );
        if ( !pModel )
        {
            HxMsgError( tr( "Không tìm thấy dữ liệu model: %1!" ).arg( pLOT->Model() ), tr( "Cài đặt không thành công" ) );
            return false;
        }

        NxDesignPtr pDesign = DesignManager()->GetDesign( pModel->Design() );
        if ( !pDesign )
        {
            HxMsgError( tr( "Không tìm thấy dữ liệu thiết kế: %1!" ).arg( pModel->Design() ), tr( "Cài đặt không thành công" ) );
            return false;
        }

        NxStopperPtr pStopper = StopperManager()->GetStopper( pModel->Stopper() );
        if ( !pStopper )
        {
            HxMsgError( tr( "Không tìm thấy dữ liệu stopper: %1!" ).arg( pModel->Stopper() ), tr( "Cài đặt không thành công" ) );
            return false;
        }

        m_pLOT = pLOT;
        m_pModel = pModel;
        m_pDesign = pDesign;
        m_pStopper = pStopper;

        try
        {
            Laser()->SetProgram( pDesign->Name() );
            PLC()->SetCvWidth( pModel->CvWidth() );
            PLC()->SetStopper( pModel->Stopper() );
        }
        catch ( NxException ex )
        {
            qApp->postEvent( qApp, new NxEvent( NxEvent::eMarkerGoError, ex.toJsonObject() ) );
            HxMsgError( ex.Message(), ex.Where() );
            return false;
        }

        qApp->postEvent( qApp, new NxEvent( NxEvent::eMarkerSetupChanged ) );
        if ( m_state == eOnFinish )
        {
            m_state = eOnFree;
            qApp->postEvent( qApp, new NxEvent( NxEvent::eMarkerGoFree ) );
        }
        return true;
    }

    return false;
}

void NxMarker::Transfer()
{
    m_state = eOnTransfering;
}

void NxMarker::Mark()
{
    m_state = eOnMarking;
}

void NxMarker::Test()
{
    m_state = eOnTesting;
}

void NxMarker::Pause()
{
    m_state = eOnFree;
    PLC()->SetEnable( false );
}

void NxMarker::Stop()
{
    m_state = eOnFree;
    PLC()->SetEnable( false );
}

NxLOTPtr NxMarker::LOT() const
{
    return m_pLOT;
}

NxModelPtr NxMarker::Model() const
{
    return m_pModel;
}

NxDesignPtr NxMarker::Design() const
{
    return m_pDesign;
}

void NxMarker::ReLoadSetting()
{
    m_settings.Load();
}

void NxMarker::CheckAndPostEvent( State& lastState, State currentState, NxEvent::Type eventType )
{
    if ( lastState == currentState )
        return;

    if ( currentState == eOnMarking )
    {
        PLC()->SetTransferMode( false );
        PLC()->SetEnable( true );
    }
    else if ( currentState == eOnTransfering )
    {
        PLC()->SetTransferMode( true );
        PLC()->SetEnable( true );
    }
    else if ( currentState == eOnFree )
    {
        PLC()->SetEnable( false );
    }
    else if ( currentState == eOnStopping )
    {
        PLC()->SetEnable( false );
    }

    qApp->postEvent( qApp, new NxEvent( eventType ) );
    lastState = currentState;
}

void NxMarker::Task()
{
    int iDebug = 0;
    m_state = eOnFree;
    State lastState = eOnUnInit;
    bool bNeedStop = false;
    while ( true )
    {
    START_LOOP:
        QThread::msleep( 20 );
        if ( bNeedStop )
            break;

        switch ( m_state )
        {
        case NxMarker::eOnFree:
            CheckAndPostEvent( lastState, m_state, NxEvent::eMarkerGoFree );
            break;
        case NxMarker::eOnTransfering:
            CheckAndPostEvent( lastState, m_state, NxEvent::eMarkerGoTransfer );
            break;
        case NxMarker::eOnFinish:
            CheckAndPostEvent( lastState, m_state, NxEvent::eMarkerGoFinish );
            break;
        case NxMarker::eOnStopping:
            CheckAndPostEvent( lastState, m_state, NxEvent::eMarkerStopped );
            bNeedStop = true;
            break;
        case NxMarker::eOnError:
            CheckAndPostEvent( lastState, m_state, NxEvent::eMarkerGoError );
            break;



        case NxMarker::eOnTesting:
        {
            CheckAndPostEvent( lastState, m_state, NxEvent::eMarkerGoTest );

            auto& positions = m_pModel->Positions();
            int patternCnt = positions.size();
            int blockCodeIndex = m_pDesign->IndexOfBlockCode();
            bool bIsSucceed = true;
            NxLOTPtr pCopyLOT = m_pLOT->Clone();
            for ( auto& [index, position] : positions )
            {
                std::map<int, QString> dataBlocks = GenMarkData( m_pDesign, pCopyLOT, m_pModel );

                try
                {
                    Laser()->SetupPosition( m_pDesign->Name(), position, m_pDesign, m_pStopper );
                    Laser()->SetupBlockData( m_pDesign->Name(), dataBlocks );
                    Laser()->Burn();
                    pCopyLOT->SetProgress( pCopyLOT->Progress() + 1 );
                    pCopyLOT->Evaluate();
                }
                catch ( NxException ex )
                {
                    // ném exception
                    QMetaObject::invokeMethod( this, [ & ]()
                                               {
                                                   HxMsgError( tr( "Không khắc được.\n%1." ).arg( ex.Message() ),
                                                               tr( "Lỗi khắc thử." ) );
                                               }, Qt::BlockingQueuedConnection );
                    qApp->postEvent( qApp, new NxEvent( NxEvent::eMarkerGoError, ex.toJsonObject() ) );
                    m_state = eOnFree;
                    bIsSucceed = false;
                    break;
                }
            }
            if ( bIsSucceed )
            {
                QMetaObject::invokeMethod( this, [ & ]()
                                           {
                                               HxMsgInfo( tr( "Đã hoàn thành khắc bản test." ),
                                                           tr( "Hoàn thành khắc." ) );
                                           }, Qt::BlockingQueuedConnection );
            }
            m_state = eOnFree;
        }
        break;



        case NxMarker::eOnMarking:
        {
            CheckAndPostEvent( lastState, m_state, NxEvent::eMarkerGoMark );

            try
            {
                if ( Barcode()->IsHasData() )
                {
                    Barcode()->Clear();
                    QString code = Barcode()->Read().trimmed();
                    if ( code.length() > 0 && code.startsWith( "ERROR" ) == false )
                    {
                        ReturnCode saveCode{};
                        QMetaObject::invokeMethod( this, [ & ]()
                                                   {
                                                       saveCode = Logger()->SaveBarcode( code );
                                                   }, Qt::BlockingQueuedConnection );
                        Barcode()->SendFeedback( true );
                    }
                    else
                    {
                        Barcode()->SendFeedback( false );
                    }
                }
            }
            catch ( NxException ex )
            {
                QMetaObject::invokeMethod( this, [ & ]()
                                           {
                                               HxMsgError( tr( "Không đọc được barcode sau khi khắc." ),
                                                           tr( "Lỗi đọc barcode." ), false );
                                           }, Qt::BlockingQueuedConnection );

                qApp->postEvent( qApp, new NxEvent( NxEvent::eMarkerGoError, ex.toJsonObject() ) );
                m_state = eOnError;
                goto START_LOOP;
            }



            try
            {
                if ( PLC()->IsHasTrigger() )
                {
                    PLC()->ConfirmTrigger();

                    auto& positions = m_pModel->Positions();
                    int patternCnt = positions.size();
                    int blockCodeIndex = m_pDesign->IndexOfBlockCode();
                    bool bIsRePrint = ( m_pLOT->IsRePrint() ) || ( m_pModel->IsPrintLo() );

                    for ( auto& [index, position] : positions )
                    {
                        m_pLOT->Evaluate();
                        if ( m_pLOT->IsCompleted() )
                        {
                            m_state = eOnFinish;
                            CheckAndPostEvent( lastState, m_state, NxEvent::eMarkerGoFinish );
                            break;
                        }

                        std::map<int, QString> dataBlocks = GenMarkData( m_pDesign, m_pLOT, m_pModel );
                        QString code = dataBlocks[ blockCodeIndex ];

                        if ( !bIsRePrint )
                        {
                            ReturnCode checkCode{};
                            QMetaObject::invokeMethod( this, [ & ]()
                                                       {
                                                           checkCode = Logger()->CheckSerialExisting( code );
                                                           if ( checkCode != RtNormal )
                                                           {
                                                               if ( checkCode == RtDBDataExisting )
                                                               {
                                                                   HxMsgError( tr( "PHÁT HIỆN TRÙNG TEM\n"
                                                                                   "LIÊN LẠC QUẢN LÝ NGAY !!!\n\n"
                                                                                   "Quá trình khắc đã dừng lại do phát hiện lỗi!" ),
                                                                               tr( "Tên LOT: %1\n"
                                                                                   "Số seri trùng: %2" )
                                                                               .arg( m_pLOT->Name() )
                                                                               .arg( code ),
                                                                               tr( "Trùng lặp tem" ), true );
                                                               }
                                                               else
                                                               {
                                                                   HxMsgError( tr( "Không kiểm tra được trùng tem\n"
                                                                                   "Quá trình khắc đã dừng lại do phát hiện lỗi!" ),
                                                                               tr( "Lỗi kiểm tra trùng tem" ) );
                                                               }
                                                           }
                                                       }, Qt::BlockingQueuedConnection );

                            // chỗ này nên ném exeption
                            if ( checkCode != RtNormal )
                            {
                                PLC()->SetMarkResult( false );
                                m_state = eOnError;
                                goto START_LOOP;
                            }

                        }

                        Laser()->SetupPosition( m_pDesign->Name(), position, m_pDesign, m_pStopper );
                        Laser()->SetupBlockData( m_pDesign->Name(), dataBlocks );
                        Laser()->Burn();

                        qApp->postEvent( qApp, new NxEvent( NxEvent::eMarkerMarked ) );

                        m_pLOT->SetProgress( m_pLOT->Progress() + 1 );
                        m_pLOT->Evaluate();

                        ReturnCode codeSaveLOT{}, savePrintCode{};
                        QMetaObject::invokeMethod( this, [ & ]()
                                                   {
                                                       codeSaveLOT = LOTManager()->Save( m_pLOT );
                                                       savePrintCode = Logger()->SavePrint( m_pLOT, m_pModel, m_pDesign, dataBlocks );

                                                       if ( savePrintCode == RtDBDataExisting )
                                                       {
                                                           HxMsgError( tr( "PHÁT HIỆN TRÙNG TEM\n"
                                                                           "LIÊN LẠC QUẢN LÝ NGAY !!!\n\n"
                                                                           "Quá trình khắc đã dừng lại do phát hiện lỗi!" ),
                                                                       tr( "Tên LOT: %1\n"
                                                                           "Số seri trùng: %2" )
                                                                       .arg( m_pLOT->Name() )
                                                                       .arg( code ),
                                                                       tr( "Trùng lặp tem" ), true );
                                                       }
                                                   }, Qt::BlockingQueuedConnection );

                        // chỗ này nên ném exeption
                        if ( savePrintCode != RtNormal )
                        {
                            PLC()->SetMarkResult( false );
                            m_state = eOnError;
                            goto START_LOOP;
                        }

                        if ( m_pLOT->IsCompleted() )
                        {
                            PLC()->SetCompleteBit();
                            m_state = eOnFinish;
                            CheckAndPostEvent( lastState, m_state, NxEvent::eMarkerGoFinish );
                            break;
                        }
                    }

                    PLC()->SetMarkResult( true );
                }
            }
            catch ( NxException ex )
            {
                QMetaObject::invokeMethod( this, [ & ]()
                                           {
                                               HxMsgError( tr( "Quá trình khắc đã dừng lại do phát hiện lỗi!" ),
                                                           tr( "Có lỗi trong quá trình khắc" ) );
                                           }, Qt::BlockingQueuedConnection );

                m_state = eOnError;
                qApp->postEvent( qApp, new NxEvent( NxEvent::eMarkerGoError, ex.toJsonObject() ) );
                m_state = eOnError;
                goto START_LOOP;
            }
        }
        break;
        default:
            break;
        }
    }
    m_state = eOnUnInit;
    qApp->postEvent( qApp, new NxEvent( NxEvent::eMarkerStopped ) );
}













//
//void NxMarker::Clear()
//{
//    lot.reset();
//    model.reset();
//    design.reset();
//    stopper.reset();
//}
//
//bool NxMarker::Select( std::shared_ptr<NxLOT> lotinf )
//{
//    //if ( lotinf == nullptr )
//    //{
//    //    NxMessage::error( "Dữ liệu lot không hợp lệ !" );
//    //    Clear();
//    //    return false;
//    //}
//
//    //if ( lotinf->IsCompleted() )
//    //{
//    //    NxMessage::error( "Lot này đã hoàn thành !" );
//    //    Clear();
//    //    return false;
//    //}
//
//    //auto _model = NxModel::Find( lotinf->modelName );
//    //if ( _model == nullptr )
//    //{
//    //    NxMessage::error( "Không tìm thấy thông tin model : " + lotinf->modelName );
//    //    Clear();
//    //    return false;
//    //}
//
//    //auto _design = NxDesign::Find( _model->design );
//    //if ( _design == nullptr )
//    //{
//    //    NxMessage::error( "Không tìm thấy thông tin thiết kế : " + _model->design );
//    //    Clear();
//    //    return false;
//    //}
//
//    //auto _stopper = NxStopper::Find( _model->stopper );
//    //if ( _stopper == nullptr )
//    //{
//    //    NxMessage::error( "Không tìm thấy thông tin stopper : " + QString::number( _model->stopper ) );
//    //    Clear();
//    //    return false;
//    //}
//
//    //// try setup
//    //try
//    //{
//    //    NxLaserDevice::SetProgram( _design->name );
//    //    PLC()->SetCvWidth( _model->cvWidth );
//    //    PLC()->SetStopper( _model->stopper );
//
//    //    lot = lotinf;
//    //    model = _model;
//    //    design = _design;
//    //    stopper = _stopper;
//    //    return true;
//    //}
//    //catch ( NxException ex )
//    //{
//    //    NxSystemError::Instance()->ErrorReport( ex );
//    //    //        Message::error(ex.message);
//    //    Clear();
//    //    return false;
//    //}
//    return false;
//}

//bool NxMarker::Mark( bool test )
//{
//    //auto tempLot = std::make_shared<NxLOT>( NxLOT() );
//    //tempLot.get()[ 0 ] = lot.get()[ 0 ];
//    //int patternCnt = model->positions.size();
//
//    //if ( test )
//    //{
//    //    for ( int i = 0; i < patternCnt; i++ )
//    //    {
//    //        NxPosition pos = model->positions[ i ];
//    //        std::map<int, QString> blockDatas = BlockDataGen ( design, tempLot, model );
//
//
//    //        NxLaserDevice::SetupPosition( design->name, pos, model->stopper, design.get()[ 0 ] );
//    //        NxLaserDevice::SetupBlockData( design->name, blockDatas );
//    //        NxLaserDevice::Burn();
//    //        tempLot->progress++;
//    //    }
//    //}
//    //else
//    //{
//    //    for ( int i = 0; i < patternCnt; i++ )
//    //    {
//    //        if ( tempLot->IsCompleted() ) continue;
//    //        NxPosition pos = model->positions[ i ];
//    //        std::map<int, QString> blockDatas = BlockDataGen( design, tempLot, model );
//
//    //        NxLaserDevice::SetupPosition( design->name, pos, model->stopper, design.get()[ 0 ] );
//    //        NxLaserDevice::SetupBlockData( design->name, blockDatas );
//    //        NxLaserDevice::Burn();
//    //        HxLogSaver::Save( tempLot, model, design );
//    //        tempLot->progress++;
//    //    }
//    //    lot.get()[ 0 ] = tempLot.get()[ 0 ];
//    //    NxLOT::SaveLot( lot );
//    //}
//    return true;
//}

//
//bool NxMarker::IsBusy()
//{
//    return runFlag;
//}
//
//void NxMarker::Start()
//{
//    if ( runFlag )
//    {
//        return;
//    }
//
//    QMetaObject::invokeMethod( this, "task" );
//}

//void NxMarker::Stop()
//{
//    runFlag = false;
//}
//
//void NxMarker::Task()
//{
//    emit started();
//    runFlag = true;
//    QThread::msleep( 500 );
//    while ( true )
//    {
//        if ( !runFlag )
//            break;
//        QThread::msleep( 50 );
//
//        try
//        {
//            if ( Barcode()->IsHasData() )
//            {
//                Barcode()->Clear();
//                QString code = Barcode()->Read().trimmed();
//                if ( code.length() > 0 && code.startsWith( "ERROR" ) == false )
//                {
//                    Barcode()->Save( code );
//                    Barcode()->SendFeedback( true );
//                }
//                else
//                {
//                    Barcode()->SendFeedback( false );
//                }
//            }
//        }
//        catch ( NxException ex )
//        {
//            ex.SetWhere( "Đọc barcode" );
//            NxSystemError::Instance()->ErrorReport( ex );
//
//            NxException ex2;
//            ex2.SetWhere( "Khắc" );
//            ex2.SetMessage( "Quá trình khắc đã dừng lại do phát hiện lỗi!" );
//            NxSystemError::Instance()->ErrorReport( ex2 );
//            runFlag = false;
//            break;
//        }
//
//        try
//        {
//            if ( PLC()->IsHasTrigger() )
//            {
//                PLC()->ConfirmTrigger();
//                bool status = Mark( false );
//                PLC()->SetMarkResult( status );
//                if ( lot->IsCompleted() )
//                {
//                    PLC()->SetCompleteBit();
//                }
//                emit printed( lot );
//            }
//        }
//        catch ( NxException ex )
//        {
//            ex.SetWhere( "Khắc" );
//            NxSystemError::Instance()->ErrorReport( ex );
//
//            NxException ex2;
//            ex2.SetWhere( "Khắc" );
//            ex2.SetMessage( "Quá trình khắc đã dừng lại do phát hiện lỗi!" );
//            NxSystemError::Instance()->ErrorReport( ex2 );
//
//            runFlag = false;
//            break;
//        }
//    }
//    runFlag = false;
//    emit stopped();
//}

NxMarker* Marker()
{
    static NxMarker instance;
    return &instance;
}