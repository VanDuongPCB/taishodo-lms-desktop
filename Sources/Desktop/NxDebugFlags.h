#pragma once
#include <QString>
#include "XDebug.h"

// Thay cho DEBUG_MODE biên dịch cứng trước đây. Giá trị lấy từ file debug của
// XDebug (<thư mục exe>/debug/lms_debug.json, đọc một lần lúc khởi động):
//
//     "plc":     "on" | "off" | "simulate"
//     "laser":   "on" | "off" | "simulate"
//     "barcode": "on" | "off" | "simulate"
//     "skip-login": true      -> mở khoá toàn bộ tab, không cần đăng nhập
//
// "off" và "simulate" đều bỏ qua phần cứng: lệnh trả về thành công ngay.
// Không có file debug thì mọi thiết bị đều "on".

inline bool IsDeviceSimulated( const char* device )
{
    const QString v = XDebug::value( device ).trimmed().toLower();
    return v == "off" || v == "simulate" || v == "sim";
}

inline bool IsPlcSimulated()     { return IsDeviceSimulated( "plc" ); }
inline bool IsLaserSimulated()   { return IsDeviceSimulated( "laser" ); }
inline bool IsBarcodeSimulated() { return IsDeviceSimulated( "barcode" ); }
inline bool IsLoginSkipped()     { return XDebug::flag( "skip-login" ); }
