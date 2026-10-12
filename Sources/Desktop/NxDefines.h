#pragma once

// DEBUG_MODE đã bỏ: dùng <thư mục exe>/debug/lms_debug.json (xem NxDebugFlags.h / XDebug).



enum ReturnCode
{
    RtNormal,

    // data
    RtDataNull,
    RtDataCannotDelete,
    RtDataNoChanges,

    // database
    RtDBFileNotFound,
    RtDBOpenFailed,
    RtDBQueryFailed,
    RtDBInsertFailed,
    RtDBDeleteFailed,
    RtDBUpdateFailed,
    RtDBSelectFailed,
    RtDBDataExisting,
};