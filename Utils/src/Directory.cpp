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
    #include <io.h>

static const size_t c_BuffLen{ 0x800 };
#else
    #include <unistd.h>
#endif

#ifdef __APPLE__
    #include <libproc.h>
#endif

namespace _Utils
{

path_t Directory::s_runPath = [] {
    path_t path;

#ifdef WIN32
    TCHAR buff[c_BuffLen];
    DWORD len = GetModuleFileName(NULL, buff, c_BuffLen);
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
#ifdef __APPLE__
    else
    {
        char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
        pid_t pid = getpid();
        int ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));
        if (ret > 0)
            path = pathbuf;
    }
#endif
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

path_t Directory::TmpPath(const std::string& appPrefix)
{
#ifdef WIN32    
    std::string dir = appPrefix;
#else
    std::string dir = appPrefix + "-" + UserName();
#endif
    auto path = std::filesystem::temp_directory_path();
    return path / dir;
}

path_t Directory::ProgramPath(const std::string& appName)
{
#ifdef WIN32
    char* env;
    size_t len;
    errno_t err = _dupenv_s(&env, &len, "ProgramFiles");
    _assert(err == 0);

    path_t path{ (env ? env : "") };
    free(env);

    path /= appName;
    if (std::filesystem::is_directory(path))
        return path;

    return s_runPath;
#else
    std::string path{ "/usr/share/" + appName };
    if (std::filesystem::is_directory(path))
        return path;
    path = "/usr/local/share/" + appName;
    if (std::filesystem::is_directory(path))
        return path;

    return s_runPath;
#endif
}

std::optional<path_t> Directory::SysCfgPath(const std::string& appName)
{
#ifdef WIN32
    char* env;
    size_t len;
    errno_t err = _dupenv_s(&env, &len, "ProgramData");
    _assert(err == 0);

    path_t path{ (env ? env : "") };
    free(env);

    path /= appName;
    if (std::filesystem::is_directory(path))
        return path;

    return std::nullopt;
#else
    std::string path{ "/etc/" + appName };
    if (std::filesystem::is_directory(path))
        return path;

    return std::nullopt;
#endif
}

std::optional<path_t> Directory::UserCfgPath(const std::string& appName, bool create)
{
#ifdef WIN32
    char* env;
    size_t len;
    errno_t err = _dupenv_s(&env, &len, "APPDATA");
    _assert(err == 0);
    
    path_t path{env ? env : ""};
    free(env);

    path /= appName;
#else
    const char* home = getenv("HOME");
    path_t path = home;
    path /= ".config/" + appName;
#endif

    if (std::filesystem::is_directory(path))
        return path;

    if (create)
    {
        std::filesystem::create_directory(path);
        if (std::filesystem::is_directory(path))
            return path;
    }

    return std::nullopt;
}

path_t Directory::UserLocalPath(const std::string& appName, bool create)
{
#ifdef WIN32
    char* env;
    size_t len;
    errno_t err = _dupenv_s(&env, &len, "LOCALAPPDATA");
    _assert(err == 0);

    path_t path{ env ? env : "" };
    free(env);

    path /= appName;

    if (std::filesystem::is_directory(path))
        return path;
    if (create)
    {
        std::filesystem::create_directory(path);
        if (std::filesystem::is_directory(path))
            return path;
    }

    return s_runPath;
#else
    const char* home = getenv("HOME");
    path_t path = home;
    path /= ".local/share/" + appName;

    if (std::filesystem::is_directory(path))
        return path;

    if (create)
    {
        std::filesystem::create_directory(path);
        if (std::filesystem::is_directory(path))
            return path;
    }

    if (s_runPath.u8string().find("/snap/") == 0)
    {
        //snap packet
        return home;
    }

    path = home;
    path /= ".local/share/";

    if (std::filesystem::is_directory(path))
        return path;

    return s_runPath;
#endif
}

std::string Directory::UserName()
{
#ifdef WIN32
    TCHAR buff[c_BuffLen];
    DWORD len = c_BuffLen;
    [[maybe_unused]]bool rc = GetUserName(buff, &len);
    _assert(rc);
    return utf8::utf16to8(reinterpret_cast<char16_t*>(buff));
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

std::string Directory::GetFileInfo(const std::filesystem::file_time_type& ftime, const uintmax_t& size, size_t size_width)
{
    std::time_t cftime = to_time_t(ftime);
    std::tm tm = *std::localtime(&cftime);

    std::stringstream sinfo;
    sinfo << std::put_time(&tm, "%d %b %Y %H:%M:%S");

    if (size > 10 * 1024 * 1024) //10M
    {
        sinfo << std::setw(size_width - 1) << size / (1024 * 1024) << " MBytes";
    }
    else if (size > 100 * 1024) //100K
    {
        sinfo << std::setw(size_width - 1) << size / 1024 << " KBytes";
    }
    else
    {
        sinfo << std::setw(size_width) << size << " Bytes";
    }

    return sinfo.str();
}

std::string Directory::GetFileInfo(const path_t& path)
{
    try 
    {
        auto ftime = std::filesystem::last_write_time(path);
        auto size = std::filesystem::file_size(path);
        return GetFileInfo(ftime, size);
    }
    catch (const std::exception& ex)
    {
        LOG(ERROR) << __FUNC__ << "exception " << ex.what();
        return {};
    }
}

fileaccess_t Directory::GetAccessMode(const path_t& path)
{
    std::error_code ec;
    if (!std::filesystem::exists(path, ec))
        return fileaccess_t::notexists;
        
    fileaccess_t mode{ fileaccess_t::exists };
    auto file = path.u8string();
#ifdef WIN32
    if (_access(file.c_str(), 4) == 0)
        mode = fileaccess_t::readonly;
    if (_access(file.c_str(), 6) == 0)
        mode = fileaccess_t::readwrite;
#else
    if (access(file.c_str(), R_OK) == 0)
        mode = fileaccess_t::readonly;
    if (access(file.c_str(), R_OK | W_OK) == 0)
        mode = fileaccess_t::readwrite;
#endif
    return mode;
}

bool DirectoryList::AddMask(const path_t& mask)
{
    //LOG(DEBUG) << "AddMask " << mask;
    std::string u8mask = mask.u8string();
    if(u8mask == "." || u8mask == "..")
        return false;
        
    m_mask = u8mask;
    m_single = true;
    m_maskList.clear();

    std::u16string m;
    auto wmask = mask.u16string();
    for (auto ch : wmask)
    {
        if (ch == ';')
        {
            m_maskList.push_back(m);
            m.clear();
        }
        else
        {
            m += std::towupper(ch);
            if (ch == '*' || ch == '?')
                m_single = false;
        }
    }
    if (m.size())
        m_maskList.push_back(m);

    if (m_maskList.size() > 1)
        m_single = false;
    
    return true;
}

bool DirectoryList::SetMask(const path_t& mask)
{
    //LOG(DEBUG) << "SetMask " << mask.u8string();

    path_t path;
    if (mask.has_parent_path())
        path = mask.u16string();
    else
        path = m_path / mask.u16string();

    std::error_code ec;
    if (!std::filesystem::is_directory(path, ec))
    {
        auto file = path.filename();
        path.remove_filename();
        if(!AddMask(file))
            path = "/";
    }

    if (!std::filesystem::is_directory(path, ec))
    {
        path = "/";
    }

    try
    {
        m_path = std::filesystem::canonical(path, ec);
        if (ec || m_path.empty())
        {
            LOG(ERROR) << __FUNC__ << " err=" << ec << " - " << ec.message();
            m_path = path.remove_filename();
        }
    }
    catch (const std::exception& ex)
    {
        LOG(ERROR) << __FUNC__ << " Exception: " << ex.what();
        m_path = "/";
    }

    //LOG(DEBUG) << "path=" << m_path.u8string() << " single mask=" << m_single;
    return true;
}

bool DirectoryList::Scan()
{
    if (m_path.empty())
        m_path = Directory::CurPath();
    //LOG(DEBUG) << "ReadDir " << m_path.u8string();

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
            m_drvList.emplace_back(d);
        }
        drv >>= 1;
    }
#else
    m_drvList.emplace_back("/");
#endif

    try
    {
        std::error_code ec;
        for (auto& entry : std::filesystem::directory_iterator(m_path, std::filesystem::directory_options::skip_permission_denied, ec))
        {
            if (entry.is_directory())
            {
                if (entry.path().filename() == "." || entry.path().filename() == "..")
                    continue;
                m_dirList.emplace_back(entry.path().filename().u8string());
            }
            else if (entry.is_regular_file())
            {
                if(m_maskList.empty())
                    m_fileList.emplace_back(entry);
                else
                {
                    for (const auto& mask : m_maskList)
                    {
                        if (Directory::MatchMask(entry.path(), mask, true))
                        {
                            m_fileList.emplace_back(entry);
                            break;
                        }
                    }
                }
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

bool DirectoryList::IsFound()
{
    if (m_fileList.size() == 1 && m_single)
        return true;
    else
        return false;
}

} //namespace _Utils
