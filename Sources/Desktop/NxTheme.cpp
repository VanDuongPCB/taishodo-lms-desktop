#include "NxTheme.h"

#include "QDebug"
#include "QDir"
#include "QFileInfo"
#include "QGraphicsDropShadowEffect"
#include "QWidget"

#include "NxSettings.h"
#include "XStyleManager.h"

namespace
{
    const char* const kEmbeddedRoot = ":/themes";
    const char* const kDefaultTheme = "Light";

    // Theme theo định dạng mới (XStyleManager): có data/variables.json và ít nhất
    // một file css khác rỗng. Thư mục Themes cũ (css rỗng, Color.json) không đạt.
    bool HasUsableTheme( const QString& root, const QString& name )
    {
        const QString dir = root + "/" + name;
        if ( !QFileInfo::exists( dir + "/data/variables.json" ) )
            return false;
        QDir css( dir + "/css" );
        if ( !css.exists() )
            return false;
        for ( const QFileInfo& fi : css.entryInfoList( { "*.css", "*.qss" }, QDir::Files ) )
            if ( fi.size() > 0 )
                return true;
        return false;
    }
}

void MakeTransparentWindow( QWidget* pWidget )
{
    if ( !pWidget )
        return;
    pWidget->setAttribute( Qt::WA_TranslucentBackground );
    auto flags = pWidget->windowFlags();
    flags |= Qt::FramelessWindowHint;
    pWidget->setWindowFlags( flags );
}

void MakeShadow( QWidget* pWidget, QPoint offset, double radius, QColor color )
{
    if ( !pWidget )
        return;
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setOffset( offset );
    shadowEffect->setBlurRadius( radius );
    shadowEffect->setColor( color );
    pWidget->setGraphicsEffect( shadowEffect );
}

NxTheme::NxTheme()
{
}

NxTheme::~NxTheme()
{
}

void NxTheme::EnsureSettings()
{
    HxRegistrySetting settings;
    QString root = settings.String( ThemeRootDir );
    QString current = settings.String( ThemeSetting );
    if ( current.isEmpty() )
        current = kDefaultTheme;

    // Thư mục ngoài chỉ được dùng khi thật sự có theme; nếu không, quay về resource.
    // (Các bản cũ lưu đường dẫn tới thư mục Themes rỗng cạnh exe.)
    bool useRoot = !root.isEmpty() && QDir( root ).exists();
    if ( useRoot && !root.startsWith( ':' ) && !HasUsableTheme( root, current ) )
        useRoot = false;
    if ( !useRoot )
        root = kEmbeddedRoot;

    XStyleManager* pStyles = XStyleManager::instance();
    pStyles->setThemeRoot( root );
    QStringList available = pStyles->availableThemes();
    if ( !available.contains( current ) )
        current = available.contains( kDefaultTheme ) ? kDefaultTheme : ( available.isEmpty() ? QString() : available.first() );

    if ( root != settings.String( ThemeRootDir ) || current != settings.String( ThemeSetting ) )
    {
        settings.Set( ThemeRootDir, root );
        settings.Set( ThemeSetting, current );
        settings.Save();
    }
}

QStringList NxTheme::GetThemes()
{
    EnsureSettings();
    return XStyleManager::instance()->availableThemes();
}

QString NxTheme::CurrentTheme() const
{
    return XStyleManager::instance()->currentTheme();
}

QString NxTheme::GetStyleSheets( const QStringList& names )
{
    return XStyleManager::instance()->styleSheet( names );
}

QColor NxTheme::GetColor( const QString& name )
{
    return XStyleManager::instance()->color( name );
}

void NxTheme::SetTheme( const QString& name )
{
    HxRegistrySetting settings;
    settings.Set( ThemeSetting, name );
    settings.Save();
    Refresh();
}

void NxTheme::Refresh()
{
    EnsureSettings();

    HxRegistrySetting settings;
    const QString root = settings.String( ThemeRootDir );
    const QString current = settings.String( ThemeSetting );

    XStyleManager* pStyles = XStyleManager::instance();
    pStyles->setThemeRoot( root );
    pStyles->setAutoApply( true );
    pStyles->setWatchEnabled( !root.startsWith( ':' ) );   // sửa nóng chỉ với thư mục ngoài

    if ( current.isEmpty() )
    {
        qWarning() << "Theme: khong co theme nao trong" << root;
        qApp->setStyleSheet( QString() );
        return;
    }
    if ( !pStyles->loadTheme( current ) )
    {
        qWarning() << "Theme: khong nap duoc" << current << "-" << pStyles->lastError();
        return;
    }
    qInfo().noquote() << QString( "Theme: %1 loaded from %2 (%3 css files, %4 variables)" )
                          .arg( current, root )
                          .arg( pStyles->styleSheetNames().size() )
                          .arg( pStyles->variables().size() );
}

NxTheme* Theme()
{
    static NxTheme instance;
    return &instance;
}
