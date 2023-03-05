﻿/*
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
#include "utils/CpConverter.h"
#include "utils/IntervalMap.h"
#include "utils/logger.h"
#include "widecharwidth/widechar_width.h"

#include <errno.h>

namespace iconvpp
{

CpConverter::CpConverter(const std::string& cp)
    : m_cp{cp}
{
    m_iconvFrom = iconv_open(s_u16.c_str(), m_cp.c_str());
    if (m_iconvFrom == s_invalidIconv)
    {
        _assert(0);
        if (errno == EINVAL)
            throw std::runtime_error{ "conversion not supported from " + m_cp };
        else
            throw std::runtime_error{ "error init conversion from " + m_cp + " errno=" + std::to_string(errno) };
    }
        
    m_iconvTo = iconv_open(m_cp.c_str(), s_u16.c_str());
    if (m_iconvTo == s_invalidIconv)
    {
        _assert(0);
        if (errno == EINVAL)
            throw std::runtime_error{ "conversion not supported to " + m_cp };
        else
            throw std::runtime_error{ "error init conversion to " + m_cp + " errno=" + std::to_string(errno) };
    }
}
    
CpConverter::~CpConverter()
{
    if (m_iconvFrom != s_invalidIconv)
        iconv_close(m_iconvFrom);
    if (m_iconvTo != s_invalidIconv)
        iconv_close(m_iconvTo);
}

bool CpConverter::Convert(std::string_view str, std::u16string& out)
{
    out.clear();
    if (m_iconvFrom == s_invalidIconv)
        return false;

    auto srcPtr = str.data();
    size_t srcSize = str.size();

    out.resize(srcSize * 2);//2x reserve
    auto outPtr = out.data();
    size_t outSize = out.size();
    size_t reserv = outSize;

    char* dstPtr = reinterpret_cast<char*>(outPtr);
    size_t dstSize = outSize * 2;

    bool rc{ true };
    while (srcSize)
    {
        size_t converted = iconv(m_iconvFrom, &srcPtr, &srcSize, &dstPtr, &dstSize);
        if (converted == static_cast<size_t>(-1))
        {
            rc = false;
            if (errno == EINVAL)
            {
                break;
            }
            else
            {
                //skip symbol
                ++srcPtr;
                --srcSize;
                *dstPtr++ = '?';
                *dstPtr++ = 0;
                dstSize -= 2;
            }
        }
    }

    //resize out str
    out.resize(reserv - dstSize / 2);
    return rc;
}

bool CpConverter::Convert(char16_t ch, std::string& out)
{
    if (m_iconvFrom == s_invalidIconv)
        return false;

    const char* srcPtr = reinterpret_cast<const char*>(&ch);
    size_t srcSize = sizeof(ch);

    out.resize(srcSize * 3); //6x reserve 
    auto dstPtr = out.data();
    size_t dstSize = out.size();
    size_t reserv = dstSize;

    size_t converted = iconv(m_iconvTo, &srcPtr, &srcSize, &dstPtr, &dstSize);
    if (converted == static_cast<size_t>(-1))
    {
        _assert(0);
        out = ' ';
        return false;
    }

    //resize out str
    out.resize(reserv - dstSize);
    return true;
}

std::list<std::string> CpConverter::GetCpList()
{
    return {
        "UTF-8", 
        "CP1250", "CP1251", "CP1252", "CP1253", "CP1254", "CP1257",
        "CP437", "CP775", "CP850", "CP852", "CP857", "CP858", "CP860", "CP863", "CP866",
        "KOI8-R"
    };
}

class WcWidth : public _Utils::IntervalMap<uint32_t, int>
{
public:
    WcWidth() : IntervalMap(1)
    { 
        AddDiaps(widechar_widened_table, widechar_widened_in_9);
        AddDiaps(widechar_unassigned_table, widechar_unassigned);
        AddDiaps(widechar_ambiguous_table, widechar_ambiguous);
        AddDiaps(widechar_doublewide_table, 2);
        AddDiaps(widechar_combining_table, widechar_combining);
        AddDiaps(widechar_combiningletters_table, widechar_combining);
        AddDiaps(widechar_nonchar_table, widechar_non_character);
        AddDiaps(widechar_nonprint_table, widechar_nonprint);
        AddDiaps(widechar_private_table, widechar_private_use);

#ifdef _DEBUG
        //check this class
        for (uint32_t i = 0; i < 0x110000; ++i)
        {
            auto v1 = operator[](i);
            auto v2 = widechar_wcwidth(i);
            _assert(v1 == v2);
        }
#endif
    }
    
    template<typename Collection>
    void AddDiaps(const Collection& diaps, int value)
    {
        for (auto& [kBegin, kEnd] : diaps)
            AddInterval(kBegin, kEnd, value);
    }
};

std::u16string CpConverter::FixPrintWidth(const std::u16string& str, size_t offset, size_t width)
{
    static WcWidth s_wcChar;

    std::u16string fixed( width, ' ');

    size_t pos{};
    for (auto c : std::u16string_view(str).substr(offset, width))
    {
        auto w = s_wcChar[c];
        if(w == 1 || w == widechar_ambiguous || c == '\x9')
            fixed[pos++] = c;
#if 0
        else if (w == 2 || w == widechar_widened_in_9)
        {
            //cann't work with width character //???
            if (pos < width - 1)
            {
                fixed[pos++] = c;
                fixed[pos++] = 0;// x200B;// 0xFEFF;  //ZERO WIDTH NO - BREAK SPACE
                --width;//???
            }
            else
                fixed[pos++] = 0x00BF; //¿ INVERTED QUESTION MARK
        }
#endif
        else
        {
            fixed[pos++] = 0x00BF; //¿ INVERTED QUESTION MARK
        }

        if (pos == width)
            break;
    }

    return fixed;
}

} //namespace iconvpp