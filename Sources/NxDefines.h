#pragma once

//#define DEBUG_MODE



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