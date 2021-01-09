/*
FreeBSD License

Copyright (c) 2020-2021 vikonix: valeriy.kovalev.software@gmail.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once
#include <filesystem>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
using path_t = std::filesystem::path;
using direntry_t = std::filesystem::directory_entry;

class Directory
{
    static path_t   m_runPath;

public:
    static std::string  m_projectName;

    static bool     SetCurDir(const std::string& path);
    static path_t   RunPath() { return m_runPath; }
    static path_t   CurPath();
    static path_t   TmpPath();
    static path_t   CfgPath();
    static path_t   SysCfgPath();
    static path_t   UserName();
    static std::string CutPath(const path_t& path, size_t len);
};


class DirectoryList
{
    std::string                 m_mask;
    path_t                      m_path;
    std::vector<std::string>    m_drvList;
    std::vector<std::string>    m_dirList;
    std::vector<direntry_t>     m_fileList;

public:
    const std::vector<std::string>& GetDrvList()  { return m_drvList; }
    const std::vector<std::string>& GetDirList()  { return m_dirList; }
    const std::vector<direntry_t>&  GetFileList() { return m_fileList; }
    const path_t&                   GetPath()     { return m_path; }

    bool    SetMask(const path_t& mask);
    bool    Scan();
};

template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
        + system_clock::now());
    return system_clock::to_time_t(sctp);
}

#if 0
enum SortOrder {
  byLast,   byName,   byExt,   bySize,   byDate,
  byLastIn, byNameIn, byExtIn, bySizeIn, byDateIn //inverce order
};

/////////////////////////////////////////////////////////////////////////////
#define MASK_NUMBER 32

class FileMask
{
protected:
  char      m_dMask[MAX_PATH + 1];
  size_t    m_nOffset1[MASK_NUMBER];
  size_t    m_nOffset2[MASK_NUMBER];

protected:
  int   Match(char* pName, char* pMask);

public:
  FileMask();

  int   SetMMask(const char* pMask);

  int   CheckFileByMask(const char* pName);
  int   CheckFileByMask(const char* pName, const char* pMask);
};

#endif