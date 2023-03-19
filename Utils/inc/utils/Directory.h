/*
FreeBSD License

Copyright (c) 2020-2023 vikonix: valeriy.kovalev.software@gmail.com
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
#include <cwctype>
#include <optional>

/////////////////////////////////////////////////////////////////////////////
namespace _Utils
{

using path_t = std::filesystem::path;
using direntry_t = std::filesystem::directory_entry;

enum class fileaccess_t
{
    notexists,
    exists,
    readonly,
    readwrite
};

class Directory
{
    static path_t   s_runPath;

    template<typename T>
    static bool Match(const T& name, const T& mask, bool nametoupper)
    {
        auto cmp_chars = [nametoupper](auto c1, auto c2) -> bool
        {
            if (!nametoupper)
                return c1 == c2;
            else
            {
                if constexpr (sizeof(c1) == 1)
                    return std::toupper(c1) == c2;// std::toupper(c2);
                else
                    return std::towupper(c1) == c2;// std::towupper(c2);
            }
        };

        // Based at algorithm written by Jack Handy - <A href="mailto:jakkhandy@hotmail.com">jakkhandy@hotmail.com</A>
        auto nameIt = name.cbegin();
        auto maskIt = mask.cbegin();

        while (nameIt != name.cend() && maskIt != mask.cend() && *maskIt != '*')
        {
            if (!cmp_chars(*nameIt, *maskIt) && *maskIt != '?')
                return false;
            ++nameIt;
            ++maskIt;
        }

        //we have * in mask
        decltype(nameIt) namePos;
        decltype(maskIt) maskPos;
        while (nameIt != name.cend() && maskIt != mask.cend())
        {
            if (*maskIt == '*')
            {
                if (++maskIt == mask.cend())
                    return true;
                namePos = nameIt + 1;
                maskPos = maskIt;
            }
            else if (cmp_chars(*nameIt, *maskIt) || *maskIt == '?')
            {
                ++nameIt;
                ++maskIt;
            }
            else
            {
                if (namePos == name.cend())
                    return false;
                nameIt = namePos++;
                maskIt = maskPos;
            }
        }

        if (nameIt != name.cend())
            return false;

        while (maskIt != mask.cend() && *maskIt == '*')
            ++maskIt;

        return maskIt == mask.cend();
    }

public:
    static bool         SetCurDir(const std::string& path);
    static path_t       CurPath();
    static path_t       RunPath() { return s_runPath; }

    static path_t                   TmpPath(const std::string& appPrefix = "");
    static path_t                   ProgramPath(const std::string& appName);
    static path_t                   UserLocalPath(const std::string& appName, bool create = false);
    static std::optional<path_t>    UserCfgPath(const std::string& appName, bool create = false);
    static std::optional<path_t>    SysCfgPath(const std::string& appName);

    static std::string  UserName();
    static std::string  CutPath(const path_t& path, size_t len);
    static std::string  GetFileInfo(const std::filesystem::file_time_type& ftime, const uintmax_t& size, size_t size_width = 8);
    static std::string  GetFileInfo(const path_t& path);
    static fileaccess_t GetAccessMode(const path_t& path);

    static bool MatchMask(const path_t& name, const path_t& mask, bool nametoupper = false)
    {
        auto nameStem = name.stem().u16string();
        auto maskStem = mask.stem().u16string();
        if(maskStem != u"*" && !Match(nameStem, maskStem, nametoupper))
            return false;

        auto nameExt  = name.extension().u16string();
        auto maskExt  = mask.extension().u16string();
        if (maskExt != u".*" && !Match(nameExt, maskExt, nametoupper))
            return false;

        return true;
    }
};


class DirectoryList
{
    std::string                 m_mask;
    std::vector<std::u16string> m_maskList;
    bool                        m_single{false};

    path_t                      m_path{"."};
    std::vector<std::string>    m_drvList;
    std::vector<std::string>    m_dirList;
    std::vector<direntry_t>     m_fileList;

public:
    const std::vector<std::string>& GetDrvList()  { return m_drvList; }
    const std::vector<std::string>& GetDirList()  { return m_dirList; }
    const std::vector<direntry_t>&  GetFileList() { return m_fileList; }
    const path_t                    GetPath()     { return m_path; }
    std::string                     GetMask()     { return m_mask; }

    bool    SetMask(const path_t& mask);
    bool    Scan();
    bool    IsFound();
    bool    IsSingleMask() { return m_single; }

protected:
    bool    AddMask(const path_t& mask);
};

template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
        + system_clock::now());
    return system_clock::to_time_t(sctp);
}

} //namespace _Utils
