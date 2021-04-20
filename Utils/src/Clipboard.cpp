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
#include "utils/Clipboard.h"
#include "utils/logger.h"
#include "utfcpp/utf8.h"

#ifdef WIN32
    #include <windows.h>

const std::u16string c_eol{ u"\r\n" };
#else
    #include <filesystem>

    namespace fs = std::filesystem;
    static const std::string s_clipFile{ "/tmp/m.clp" };
#endif

namespace _Utils
{

bool CopyToClipboard(const std::vector<std::u16string>& strArray, bool eol)
{
    //LOG(DEBUG) << "CopyToClipboard";

#ifdef WIN32
    // Open the clipboard, and empty it.
    if(!OpenClipboard(NULL))
        return false;
    EmptyClipboard();
    
    std::u16string str;
    for (auto& s : strArray)
    {
        str += s;
        str += c_eol;
    }
    if (eol)
        str += c_eol;

    //copy text using the CF_UNICODETEXT format.
    // Allocate a global memory object for the text.
    HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, str.size() * 2 + 2);
    if(hglbCopy == NULL)
    {
        CloseClipboard();
        return false;
    }

    // Lock the handle and copy the text to the buffer.
    void* data = GlobalLock(hglbCopy);
    if(!data)
    {
        CloseClipboard();
        return false;
    }

    std::memcpy(data, str.c_str(), str.size() * 2 + 2);
    GlobalUnlock(hglbCopy);

    // Place the handle on the clipboard.
    SetClipboardData(CF_UNICODETEXT, hglbCopy);
    CloseClipboard();
#else

    std::ofstream file{ s_clipFile, std::ios::trunc };
    if (!file)
        return false;

    fs::permissions(s_clipFile,
        fs::perms::group_write | fs::perms::others_write,
        fs::perm_options::add);

    for (auto& s : strArray)
    {
        file << utf8::utf16to8(s) << std::endl;
    }
    if (eol)
        file << std::endl;
#endif

    return true;
}

bool PasteFromClipboard(std::vector<std::u16string>& strArray)
{
    //LOG(DEBUG) << "PasteFromClipboard";

#ifdef WIN32
    if(!IsClipboardFormatAvailable(CF_UNICODETEXT))
        return false;

    if(!OpenClipboard(NULL))
        return false;

    HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
    if (hglb == NULL)
    {
        CloseClipboard();
        return false;
    }

    void* paste = GlobalLock(hglb);
    if(paste == NULL)
    {
        CloseClipboard();
        return false;
    }

    strArray.clear();
    std::u16string_view str{ static_cast<char16_t*>(paste) };
    size_t offset = 0;
    while (offset < str.size())
    {
        size_t strEnd = str.find(c_eol, offset);
        strArray.push_back(std::u16string{ str.substr(offset, strEnd - offset) });
        if (strEnd == std::string::npos)
            break;
        offset = strEnd + 2;
    }
    
    GlobalUnlock(hglb);
    CloseClipboard();
#else

    if (!IsClipboardReady())
        return false;

    std::ifstream file{ s_clipFile };
    if (!file)
        return false;

    std::string str;
    while (std::getline(file, str))
        strArray.push_back(utf8::utf8to16(str));

#endif
    return true;
}


bool IsClipboardReady()
{
    //LOG(DEBUG) << "IsClipboardReady";

#ifdef WIN32
    return IsClipboardFormatAvailable(CF_UNICODETEXT);
#else

    std::error_code ec;
    if (fs::is_regular_file(s_clipFile, ec) && fs::file_size(s_clipFile, ec) > 0)
        return true;
    else
        return false;
#endif
}

} //namespace _Utils
