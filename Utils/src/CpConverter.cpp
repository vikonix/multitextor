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

#include "utils/CpConverter.h"
#include "utils/logger.h"

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
        "CP437", "CP866", 
        "CP1251"
    };
}

} //namespace iconvpp