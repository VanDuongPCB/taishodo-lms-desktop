#pragma once
#include "QString"
#include "QSettings"

constexpr auto DatabaseFilePath         = "Data/DatabaseFilePath";
constexpr auto DataRootDir              = "Data/DataRootDir";
constexpr auto IVProgramDir             = "Data/IVProgramDir";
constexpr auto ThemeRootDir             = "Theme/RootDir";
constexpr auto ThemeSetting             = "Theme/Current";
constexpr auto AdminPassword            = "Protect/AdminPassword";
constexpr auto SuperPassword            = "Protect/SuperPassword";
constexpr auto LeaderPassword           = "Protect/LeaderPassword";
constexpr auto LaserConnPort            = "Hardware/LaserConnPort";
constexpr auto PLCConnIPAddress         = "Hardware/PLCConnIPAddress";
constexpr auto PLCConnPort              = "Hardware/PLCConnPort";
constexpr auto RegCvWidth               = "Hardware/RegCvWidth";
constexpr auto RegStopper               = "Hardware/RegStopper";
constexpr auto RegPassMode              = "Hardware/RegPassMode";
constexpr auto RegSoftwareReady         = "Hardware/RegSoftwareReady";
constexpr auto RegLaserTrigger          = "Hardware/RegLaserTrigger";
constexpr auto RegLaserTriggerConfirm   = "Hardware/RegLaserTriggerConfirm";
constexpr auto RegLaserStop             = "Hardware/RegLaserStop";
constexpr auto RegLaserStopConfirm      = "Hardware/RegLaserStopConfirm";
constexpr auto RegBarcodeHasData        = "Hardware/RegBarcodeHasData";
constexpr auto RegBarcodeConfirm        = "Hardware/RegBarcodeConfirm";
constexpr auto RegBarcodeData           = "Hardware/RegBarcodeData";
constexpr auto RegBarcodeOK             = "Hardware/RegBarcodeOK";
constexpr auto RegBarcodeNG             = "Hardware/RegBarcodeNG";
constexpr auto RegMarkingResult         = "Hardware/RegMarkingResult";

class HxRegistrySetting : private QSettings
{
public:
    explicit HxRegistrySetting();
    HxRegistrySetting& operator = ( const HxRegistrySetting& );

    bool Bool( QAnyStringView key );
    int Int( QAnyStringView key );
    double Double( QAnyStringView key );
    QString String( QAnyStringView key );

    void Set( QAnyStringView key, const QVariant& value );
    void Clear();
    void Load();
    void Save();
private:
    QString m_regPath;
    std::map<QAnyStringView, QVariant> m_defaultValues;
    std::map<QAnyStringView, QVariant> m_dataLoaded;
    std::map<QAnyStringView, QVariant> m_dataToSave;
};
