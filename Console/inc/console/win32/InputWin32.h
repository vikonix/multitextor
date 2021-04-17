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
#ifdef WIN32

#include "Console/ConsoleInput.h"
#include <windows.h>

#include <atomic>

//////////////////////////////////////////////////////////////////////////////
class InputWin32 final : public ConsoleInput
{
    HANDLE  m_hStdin { INVALID_HANDLE_VALUE };
    HWND    m_hWnd { NULL };
    cp_t    m_ioCP {0};
    
    //mouse input
    bool    m_fMouseTrack { false };
    bool    m_prevKeyUp { false };
    pos_t   m_prevX { 0x100 };
    pos_t   m_prevY { 0x100 };
    input_t m_prevKey { 0 };
    clock_t m_prevTime { 0 };

protected:
    static std::atomic_bool s_fCtrlC;

public:
    InputWin32() = default;
    virtual ~InputWin32() { Deinit(); }

    virtual bool Init() override final;
    virtual void Deinit() override  final;
    virtual bool InputPending(const std::chrono::milliseconds& WaitTime = 500ms) override  final;

    virtual bool SwitchToStdConsole() override final;
    virtual bool RestoreConsole() override final;

protected:
    static BOOL InputWin32::CtrlHandler(DWORD fdwCtrlType);

    void ProcessInput();
    void ProcessKeyEvent   (KEY_EVENT_RECORD*           pKeyEvent);
    void ProcessMouseEvent (MOUSE_EVENT_RECORD*         pMouseEvent);
    void ProcessResizeEvent(WINDOW_BUFFER_SIZE_RECORD*  pWindowBufferSizeEvent);
    void ProcessFocusEvent (FOCUS_EVENT_RECORD*         pFocusEvent);
    void ProcessMenuEvent  (MENU_EVENT_RECORD*          pMenuEvent);

    void WriteResize(pos_t x, pos_t y);
};

#endif //WIN32