#pragma once
#include "QApplication"
#include "QColor"
#include "QObject"
#include "QPoint"
#include "QString"
#include "QStringList"

void MakeTransparentWindow( QWidget* pWidget );
void MakeShadow( QWidget* pWidget, QPoint offset = QPoint( 0, 0 ), double radius = 20, QColor color = Qt::black );

// Lớp mỏng trên XStyleManager (xQt), giữ API cũ của ứng dụng.
// Theme nằm trong resource ":/themes/<Tên>" (Sources/Resources/themes, sinh bởi
// tools/generate_theme.py, nhúng qua Sources/Resources/themes.qrc). Registry
// "Theme/RootDir" có thể trỏ sang thư mục ngoài cùng cấu trúc để sửa theme nóng;
// "Theme/Current" là tên theme đang dùng (mặc định "Light").
class NxTheme : public QObject
{
    Q_OBJECT
public:
    NxTheme();
    ~NxTheme();

    QStringList GetThemes();                                 // tên các theme có trong root hiện tại
    QString CurrentTheme() const;
    QString GetStyleSheets( const QStringList& names = {} ); // toàn bộ hoặc theo tên file ("messagebox")
    QColor GetColor( const QString& name );                  // biến màu trong data/variables.json

    void SetTheme( const QString& name );                    // lưu registry và áp dụng
    void Refresh();                                          // nạp lại theme theo registry

private:
    void EnsureSettings();                                   // root/current hợp lệ, mặc định resource + Light
};

NxTheme* Theme();
