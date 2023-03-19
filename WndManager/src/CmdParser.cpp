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
#include "Console/KeyCodes.h"
#include "WndManager/CmdParser.h"
#include "WndManager/App.h"

namespace _WndManager
{

bool CmdParser::SetCmdMap(const CmdMap& cmdMap)
{
    m_keyMap.clear();
    m_cmdMap.clear();
    m_savedKeys.clear();

    for (auto& [keys, cmd] : cmdMap)
    {
        m_keyMap.push_back(keys);
        m_cmdMap.push_back(cmd);
    }
    
    return true;
}

scancmd_t CmdParser::ScanKey(input_t key)
{
    if(m_keyMap.empty())
        return scancmd_t::not_found;
    
    if ((key & K_TYPEMASK) >= K_MOUSE && (key & K_TYPEMASK) <= K_MOUSEWDN)
        //cut mouse coordinates info
        key &= (K_TYPEMASK | K_MODMASK);
    else if ((key & K_TYPEMASK) == K_SYMBOL && (key & (K_ALT | K_CTRL)) == 0)
        //cut shift key info
        key &= ~K_SHIFT;
    else if (key == K_TIME)
    {
        if (m_savedKeys.empty())
            return scancmd_t::not_found;
        else
        {
            if (time(NULL) - m_time > 1)
            {
                //long time between key pressing
                //return all collected codes
                Application::getInstance().StatusWaitKey();
                return scancmd_t::collected;
            }
            else
            {
                return scancmd_t::wait_next;
            }
        }
    }
    
    m_savedKeys.push_back(key);
    auto savedptr = m_savedKeys.data();
    
    auto cmdit = m_cmdMap.cbegin();
    for (const auto& keylist : m_keyMap)
    {
        ++cmdit;
        if (keylist.size() < m_savedKeys.size())
            continue;
        
        auto keyptr = keylist.data();
        if (keylist.size() == m_savedKeys.size())
        {
            if (!std::memcmp(savedptr, keyptr, m_savedKeys.size() * sizeof(input_t)))
            {
                --cmdit;
                m_savedKeys = *cmdit;
                Application::getInstance().StatusWaitKey();
                return scancmd_t::collected;
            }
        }
        else
        {
            if (!std::memcmp(savedptr, keyptr, m_savedKeys.size() * sizeof(input_t)))
            {
                m_time = time(NULL);
                if(m_savedKeys.size() == 1) 
                    Application::getInstance().StatusWaitKey(true);
                return scancmd_t::wait_next;
            }
        }
    }

    if (m_savedKeys.size() == 1)
    {
        m_savedKeys.clear();
        return scancmd_t::not_found;
    }
    else
        return scancmd_t::collected;
}

std::vector<input_t>&& CmdParser::GetCommand()
{
    //return and clear saved keys
    return std::move(m_savedKeys);
}

} //namespace _WndManager 
