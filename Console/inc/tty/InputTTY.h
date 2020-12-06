/*
FreeBSD License

Copyright (c) 2020 vikonix: valeriy.kovalev.software@gmail.com
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
#ifndef WIN32

#include "ConsoleInput.h"
#include "utils/logger.h"
#include "tty/TermcapMap.h"

#include <termios.h>
#include <time.h>

#include <atomic>
#include <unordered_map>
#include <iomanip>


#define K_CHAR 0xfffe

//////////////////////////////////////////////////////////////////////////////
class KeyMapper
{
    size_t m_maxSeqSize {0};
    std::unordered_map<std::string, input_t> m_map;

public:
    bool AddKey(const std::string& sequence, input_t code)
    {
        try
        {
            m_maxSeqSize = std::max(m_maxSeqSize, sequence.size());
            m_map.emplace(sequence, code);
            LOG(DEBUG) << "add seq=" << CastEscString(sequence) << " " << std::hex << ConsoleInput::CastKeyCode(code) << std::dec;
        }
        catch(...)
        {
            return false;
        }
        return true;
    }
    
    bool ChangeKey(const std::string& sequence, input_t code)
    {
        try
        {
            auto item = m_map.find(sequence);
            if(item != m_map.end())
                item->second = code;
        }
        catch(...)
        {
            return false;
        }
        return true;
    }
    
    input_t GetCode(const std::string& sequence)
    {
        if(sequence.size() > m_maxSeqSize)
            return K_ERROR;
        
        for(auto&& [seq, code] : m_map)
        {
            auto pos = seq.find(sequence);
            if(pos == 0)
            {
                if(seq.size() == sequence.size())
                {
                    LOG(DEBUG) << "seq=" << CastEscString(sequence) << " code=" << std::hex << code << std::dec;
                    return code;
                }
                else
                {
                    return K_UNUSED;
                }
            }
        }

        return K_ERROR;
    }
};

//////////////////////////////////////////////////////////////////////////////
class InputTTY final: public ConsoleInput
{
    inline static const size_t MaxInputLen {32};
    
    static std::atomic_bool s_fResize;
    static std::atomic_bool s_fCtrlC;

    const TermcapBuffer&  m_termcap {TermcapBuffer::getInstance()};
    
    struct termios  m_termnew;
    struct termios  m_termold;
    
    KeyMapper       m_KeyMap;

    int             m_stdin {-1};
    bool            m_fTerm{false};
    bool            m_fTiocLinux{false}; //linux only
    input_t         m_prevMode{K_UNUSED};   //linux only

    pos_t           m_prevX {0xff};
    pos_t           m_prevY {0xff};
    input_t         m_prevKey {0};
    clock_t         m_prevTime {0};
    bool            m_prevUp {false};

public:
    InputTTY() = default;
    virtual ~InputTTY() { Deinit(); }

    virtual bool    Init() override final;
    virtual void    Deinit() final;
    virtual bool    InputPending(const std::chrono::milliseconds& WaitTime = 500ms) override  final;

    virtual bool    SwitchToStdConsole() override final;
    virtual bool    RestoreConsole() override final;

private:
    static void     Abort(int signal);
    static void     Resize(int signal);
    static void     CtrlC(int signal);
    static void     Child(int signal);

    bool            LoadKeyCode();
    bool            InitSignals();
    std::string     GetConsoleCP();

    size_t          ReadConsole(std::string& str, size_t n);
    input_t         ProcessMouse(pos_t x, pos_t y, input_t k);
    void            ProcessInput(bool fMouse = false);
    void            ProcessSignals();
};

#endif //!WIN32
