#pragma once
#include "vector"
#include "memory"
#include "QString"
#include "QDateTime"

struct NxIVProgram
{
    QString Name;
    QString FilePath;
    QDateTime LastTimeModified;
    size_t FileSize;
};

using NxIVProgramArray = std::vector<NxIVProgram>;
class NxIVProgramManager
{
public:
    NxIVProgramArray Get();
};


NxIVProgramManager* IVProgram();