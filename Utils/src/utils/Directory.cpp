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
#include "utils/Directory.h"
#include "utils/logger.h"
#include "utfcpp/utf8.h"

#ifdef WIN32
    #include <windows.h>
    static const size_t c_BuffLen = 0x800;
#endif

#include <cwctype>


std::string  Directory::m_projectName;
path_t Directory::m_runPath = [] {
    path_t path;

#ifdef WIN32
    TCHAR buff[c_BuffLen];
    DWORD len = GetModuleFileName(NULL, buff, sizeof(buff) / sizeof(TCHAR));
    buff[len] = 0;
    _assert(len > 0);
    path = buff;
#else
    //linux bsd
    const std::string self{ "/proc/self/exe" };
    if (std::filesystem::is_symlink(self))
    {
        path = std::filesystem::read_symlink(self);
    }
    else
    {
        LOG(ERROR) << "self link not exists";
    }
#endif

    path.remove_filename();
    return path;
}();


bool Directory::SetCurDir(const std::string& path)
{
    std::error_code rc;
    std::filesystem::current_path(path, rc);
    return !rc;
}

path_t Directory::CurPath()
{
    auto path = std::filesystem::current_path();
    return path;
}

path_t Directory::TmpPath()
{
    auto path = std::filesystem::temp_directory_path();
    return path;
}

path_t Directory::CfgPath()
{
#ifdef WIN32
    return m_runPath;
#else
    std::string path{ "/etc/" + m_projectName };
    if (std::filesystem::is_directory(path))
        return path;

    return m_runPath;
#endif
}

path_t Directory::SysCfgPath()
{
#ifdef WIN32
    TCHAR buff[c_BuffLen];
    [[maybe_unused]]bool rc = GetWindowsDirectory(buff, sizeof(buff) / sizeof(TCHAR));
    _assert(rc);
    return buff;
#else
    return "/etc";
#endif
}

path_t Directory::UserName()
{
#ifdef WIN32
    TCHAR buff[c_BuffLen];
    DWORD len = sizeof(buff) / sizeof(TCHAR);
    [[maybe_unused]]bool rc = GetUserName(buff, &len);
    _assert(rc);
    return buff;
#else
    const char* user = getenv("USER");
    if (!user || 0 == *user)
        user = getenv("USERNAME");
    if (!user || 0 == *user)
        user = getenv("LOGNAME");
    if (!user || 0 == *user)
        user = "usr";
    std::string name{ user };
    return name;
#endif
}

std::string Directory::CutPath(const path_t& path, size_t len)
{
    auto wstr = path.u16string();
    if (wstr.size() < len)
        return path.u8string();
    
    auto shortPath = path.root_path() / "..~";
    size_t offset = wstr.size() - (len - shortPath.u16string().size());
    shortPath += wstr.substr(offset, std::string::npos);
    return shortPath.u8string();
}

bool DirectoryList::SetMask(const path_t& mask)
{
    auto path = m_path / mask.u16string();
    try
    {
        m_path = std::filesystem::canonical(path.u16string());
    }
    catch (const std::exception& ex)
    {
        LOG(ERROR) << __FUNC__ << " Exception: " << ex.what();
    }

    if (!std::filesystem::is_directory(m_path))
    {
        auto file = m_path.filename();
        m_path.remove_filename();
        m_mask = file.u8string();
    }
    else
        m_mask.clear();

    if (!std::filesystem::is_directory(m_path))
    {
        m_path = "/";
    }

    return true;
}

bool DirectoryList::Scan()
{
    if (m_path.empty())
        m_path = Directory::CurPath();
    LOG(DEBUG) << "ReadDir " << m_path;

    m_drvList.clear();
    m_dirList.clear();
    m_fileList.clear();

#ifdef WIN32
    DWORD drv = GetLogicalDrives();
    for (char i = 0; i < 32 && drv; ++i)
    {
        if (0 != (drv & 1))
        {
            std::string d;
            d = 'a' + i;
            d += ":\\";
            m_drvList.push_back(std::move(d));
        }
        drv >>= 1;
    }
#else
    m_drvList.emplace_back("/");
#endif

    try
    {
        for (auto& entry : std::filesystem::directory_iterator(m_path))
        {
            if (entry.is_directory())
            {
                if (entry.path().filename() == "." || entry.path().filename() == "..")
                    continue;
                m_dirList.push_back(entry.path().filename().u8string());
            }
            else if (entry.is_regular_file())
            {
                m_fileList.push_back(entry);
            }
        }
    }
    catch (const std::exception& ex)
    {
        LOG(ERROR) << __FUNC__ << " Exception: " << ex.what();
    }

    std::sort(m_dirList.begin(), m_dirList.end(), [](const std::string& str1, const std::string& str2) {
        std::u16string wstr1 = utf8::utf8to16(str1);
        std::u16string wstr2 = utf8::utf8to16(str2);
        std::transform(wstr1.begin(), wstr1.end(), wstr1.begin(),
            [](char16_t c) -> char16_t { return std::towupper(c); });
        std::transform(wstr2.begin(), wstr2.end(), wstr2.begin(),
            [](char16_t c) -> char16_t { return std::towupper(c); });
        return wstr1 < wstr2;
        });

    std::sort(m_fileList.begin(), m_fileList.end(),[](const direntry_t& entry1, const direntry_t& entry2) {
        std::u16string wstr1 = entry1.path().u16string();
        std::u16string wstr2 = entry2.path().u16string();
        std::transform(wstr1.begin(), wstr1.end(), wstr1.begin(),
            [](char16_t c) -> char16_t { return std::towupper(c); });
        std::transform(wstr2.begin(), wstr2.end(), wstr2.begin(),
            [](char16_t c) -> char16_t { return std::towupper(c); });
        return wstr1 < wstr2;
        });

    if(m_path.has_parent_path())
        m_dirList.insert(m_dirList.begin(), "..");

    return true;
}

