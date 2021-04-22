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

#include "Types.h"
#include "KeyCodes.h"

#include <list>
#include <string>
#include <functional>
#include <atomic>

namespace _Console
{

using keybuff_t = std::list<input_t>;

class InputBuffer
{
protected:
    keybuff_t m_keyBuff;
    keybuff_t m_macroBuff;

public:
    bool PutInput(const input_t Code)
    {
        try
		{
            m_keyBuff.push_back(Code);
        }
        catch(...)
		{
            return false;
        }
        return true;
    }

    bool PutInput(const keybuff_t& keyBuff)
    {
	    try
		{
            m_keyBuff.insert(m_keyBuff.end(), keyBuff.begin(), keyBuff.end());
        }
        catch(...)
		{
            return false;
        }
        return true;
    }

    input_t GetInput()
    {
        if(m_keyBuff.empty())
            return 0;

        input_t code = m_keyBuff.front();
        m_keyBuff.pop_front();
        return code;
    }

    size_t GetInputLen()
    {
        return m_keyBuff.size();
    }

    void ClearMacro()
    {
        m_macroBuff.clear();
    }

    bool PutMacro(const input_t Code)
    {
        try
		{
            m_macroBuff.push_back(Code);
        }
        catch(...)
		{
            return false;
        }
        return true;
    }

    bool PlayMacro()
    {
        try 
        {
            m_keyBuff.insert(m_keyBuff.end(), m_macroBuff.begin(), m_macroBuff.end());
        }
        catch(...) 
        {
            return false;
        }
        return true;
    }
};

using ResizeFunction = std::function<bool(pos_t& x, pos_t& y)>;

//////////////////////////////////////////////////////////////////////////////
class ConsoleInput : public InputBuffer
{
friend class Console;

protected:
    ResizeFunction m_ResizeCallback {nullptr};

public:
    static std::atomic_bool s_fExit;

    virtual bool Init() = 0;
    virtual void Deinit() = 0;
    virtual bool InputPending(const std::chrono::milliseconds& WaitTime = 500ms) = 0;

    static std::string CastKeyCode(input_t code);
};

} //namespace _Console
