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
#ifdef WIN32

#include "Console/win32/InputWin32.h"
#include "utils/logger.h"

namespace _Console
{

std::atomic_bool ConsoleInput::s_fExit {false};
std::atomic_bool InputWin32::s_fCtrlC {false};

//////////////////////////////////////////////////////////////////////////////
bool InputWin32::Init()
{
    m_fMouseTrack = false;

    if(INVALID_HANDLE_VALUE != m_hStdin)
        return true;

    // stdin may have been redirected. So try this
    m_hStdin = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == m_hStdin)
    {
        LOG(ERROR) << __FUNC__ << "CreateFile(CONIN$...) error" << GetLastError();
        return false;
    }

    if (!GetConsoleMode(m_hStdin,  &m_saveMode))
    {
        LOG(ERROR) << __FUNC__ << "GetConsoleMode error" << GetLastError();
        Deinit();
        return false;
    }

    DWORD dwModeIn = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    if (!SetConsoleMode(m_hStdin, dwModeIn))
    {
        LOG(ERROR) << __FUNC__ << "SetConsoleMode error" << GetLastError();
        Deinit();
        return false;
    }

    if(!SetConsoleCtrlHandler(
        (PHANDLER_ROUTINE) CtrlHandler, // handler function
        TRUE))                          // add to list
    {
        LOG(ERROR) << __FUNC__ << "SetConsoleCtrlHandler error" << GetLastError();
        Deinit();
        return false;
    }

    m_ioCP = GetConsoleCP();
    LOG(INFO) << "cp=" << m_ioCP;

    char* pLC_CTYPE = setlocale(LC_CTYPE, "");
    LOG(INFO) << "LC_CTYPE=" << pLC_CTYPE;

    HWND wnd = GetConsoleWindow();

    m_hWnd = GetForegroundWindow();
    LOG(INFO) << "hWnd=" << m_hWnd << " consolewnd=" << wnd;
    
    SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);
    return true;
}

void InputWin32::Deinit()
{
    if (INVALID_HANDLE_VALUE == m_hStdin)
        return;

    CloseHandle(m_hStdin);
    m_hStdin = INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////////////
BOOL InputWin32::CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
    // Handle the CTRL+C signal.
    case CTRL_C_EVENT:
        //LOG(INFO) << "signal Ctrl+C";
        //ignore
        s_fCtrlC = true;
        return TRUE;

    case CTRL_BREAK_EVENT:
        LOG(INFO) << "signal Ctrl+Break";
        s_fExit = true;
        Sleep(2000);
        return TRUE;

    // CTRL+CLOSE: confirm that the user wants to exit.
    case CTRL_CLOSE_EVENT:
        LOG(INFO) << "signal Close";
        s_fExit = true;
        Sleep(2000);
        return TRUE;

    // Pass other signals to the next handler.
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    default:
        s_fExit = true;
        LOG(INFO) << "signal Other " << fdwCtrlType;
        Sleep(2000);
        return FALSE;
    }
}

//////////////////////////////////////////////////////////////////////////////
void InputWin32::WriteResize(pos_t x, pos_t y)
{
    DWORD n;
    INPUT_RECORD rec;

    rec.EventType = WINDOW_BUFFER_SIZE_EVENT;
    rec.Event.WindowBufferSizeEvent.dwSize.X = x;
    rec.Event.WindowBufferSizeEvent.dwSize.Y = y;

    WriteConsoleInput(m_hStdin, &rec, 1, &n);
}

//////////////////////////////////////////////////////////////////////////////
bool InputWin32::InputPending(const std::chrono::milliseconds& WaitTime)
{
    ULONGLONG ms = WaitTime.count();

    if(INVALID_HANDLE_VALUE == m_hStdin)
        return false;

    if(0 != GetInputLen())
        return true;

    ProcessInput();
    while(ms && 0 == GetInputLen() && !s_fExit)
    {
        ULONGLONG t = GetTickCount64();

        WaitForSingleObject(m_hStdin, static_cast<DWORD>(ms));
        ProcessInput();

        ULONGLONG dt = GetTickCount64() - t;
        ms = (ms > dt) ? ms - dt : 0;
    }

    return 0 != GetInputLen();
}

//////////////////////////////////////////////////////////////////////////////
void InputWin32::ProcessInput()
{
    if (s_fExit)
    {
        PutInput(K_EXIT);
        return;
    }

    if (s_fCtrlC)
    {
        s_fCtrlC = false;
        PutInput('C' | K_CTRL);
    }

    DWORD count;

    if (!GetNumberOfConsoleInputEvents(m_hStdin, &count))
        return;

    while (count-- > 0)
    {
        INPUT_RECORD record;
        DWORD n;
        BOOL rc = PeekConsoleInput(m_hStdin, &record, 1, &n);
        if (rc && 0 != n)
        {
            rc = ReadConsoleInput(m_hStdin, &record, 1, &n);

            switch (record.EventType)
            {
            case KEY_EVENT:
                ProcessKeyEvent(&record.Event.KeyEvent);
                break;

            case MOUSE_EVENT:
                ProcessMouseEvent(&record.Event.MouseEvent);
                break;

            case WINDOW_BUFFER_SIZE_EVENT:
                ProcessResizeEvent(&record.Event.WindowBufferSizeEvent);
                break;

            case FOCUS_EVENT:
                ProcessFocusEvent(&record.Event.FocusEvent);
                break;

            case MENU_EVENT:
                ProcessMenuEvent(&record.Event.MenuEvent);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
void InputWin32::ProcessKeyEvent(KEY_EVENT_RECORD* pKeyEvent)
{
/*    
    LOG(DEBUG) << "KeyEvent n=" << pKeyEvent->wRepeatCount
        << " vk=" << std::hex << pKeyEvent->wVirtualKeyCode
        << " scan=" << pKeyEvent->wVirtualScanCode
        << " char=" << std::hex << +pKeyEvent->uChar.AsciiChar << ":" << pKeyEvent->uChar.UnicodeChar
        << " ctrl=" << std::hex << pKeyEvent->dwControlKeyState
        << " down=" << pKeyEvent->bKeyDown << std::dec
        ;
//*/
    if(!pKeyEvent->bKeyDown)
    {
        //check release key
        switch(pKeyEvent->wVirtualKeyCode)
        {
        case VK_SHIFT:
            if((pKeyEvent->dwControlKeyState & SHIFT_PRESSED) == 0)
                PutInput(K_RELEASE | K_SHIFT);
            break;
        }
        return;
    }

    WCHAR   wc = pKeyEvent->uChar.UnicodeChar;
    size_t  n = pKeyEvent->wRepeatCount;
    input_t iKey = 0;
    input_t iKeyMode = 0;

    if(pKeyEvent->dwControlKeyState & SHIFT_PRESSED)
        iKeyMode |= K_SHIFT;
    if(pKeyEvent->dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
        iKeyMode |= K_CTRL;
    if(pKeyEvent->dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
        iKeyMode |= K_ALT;

    switch(pKeyEvent->wVirtualKeyCode)
    {
    case VK_F1:
        iKey = K_F1;
        break;
    case VK_F2:
        iKey = K_F2;
        break;
    case VK_F3:
        iKey = K_F3;
        break;
    case VK_F4:
        iKey = K_F4;
        break;
    case VK_F5:
        iKey = K_F5;
        break;
    case VK_F6:
        iKey = K_F6;
        break;
    case VK_F7:
        iKey = K_F7;
        break;
    case VK_F8:
        iKey = K_F8;
        break;
    case VK_F9:
        iKey = K_F9;
        break;
    case VK_F10:
        iKey = K_F10;
        break;
    case VK_F11:
        iKey = K_F11;
        break;
    case VK_F12:
        iKey = K_F12;
        break;

    case VK_UP:
        iKey = K_UP;
        break;
    case VK_DOWN:
        iKey = K_DOWN;
        break;
    case VK_LEFT:
        iKey = K_LEFT;
        break;
    case VK_RIGHT:
        iKey = K_RIGHT;
        break;

    case VK_PRIOR:
        iKey = K_PAGEUP;
        break;
    case VK_NEXT:
        iKey = K_PAGEDN;
        break;
    case VK_HOME:
        iKey = K_HOME;
        break;
    case VK_END:
        iKey = K_END;
        break;
    case VK_INSERT:
        iKey = K_INSERT;
        break;
    case VK_DELETE:
        iKey = K_DELETE;
        break;

    case VK_ESCAPE:
        iKey = K_ESC;
        break;
    case VK_TAB:
        iKey = K_TAB;
        break;
    case VK_RETURN:
        iKey = K_ENTER;
        break;
    case VK_BACK:
        iKey = K_BS;
        break;
    }

    if(!iKey && wc)
    {
        if(wc < ' ')
        {
            iKey = 'A' - 1 + wc;
            iKeyMode |= K_CTRL;
        }
        else
        {
            if((iKeyMode & (K_CTRL | K_ALT)) == 0)
            {
                //simbol code
                iKey = wc;
            }
            else
            {
                //key combination
                if((pKeyEvent->wVirtualKeyCode >= '0' && pKeyEvent->wVirtualKeyCode <= '9')
                || (pKeyEvent->wVirtualKeyCode >= 'A' && pKeyEvent->wVirtualKeyCode <= 'Z'))
                    iKey = pKeyEvent->wVirtualKeyCode;
                else
                    iKey = wc;
            }
        }
    }

    if(iKey)
    {
        iKey |= iKeyMode;
        while(n--)
            PutInput(iKey);
    }
}

//////////////////////////////////////////////////////////////////////////////
void InputWin32::ProcessMouseEvent(MOUSE_EVENT_RECORD* pMouseEvent)
{
/*    
    LOG_IF(pMouseEvent->dwButtonState, DEBUG) 
        << std::hex << "MouseEvent key=" << pMouseEvent->dwButtonState << std::dec
        << " x=" << pMouseEvent->dwMousePosition.X
        << " y=" << pMouseEvent->dwMousePosition.Y
        << " ctrl=" << std::hex << pMouseEvent->dwControlKeyState
        << " event=" << pMouseEvent->dwEventFlags << std::dec
        ;
//*/
    pos_t x = pMouseEvent->dwMousePosition.X;
    pos_t y = pMouseEvent->dwMousePosition.Y;

    bool prevUp = m_prevKeyUp;

    if(!pMouseEvent->dwButtonState)
    {
        if(!m_fMouseTrack)
            return;

        m_fMouseTrack = false;
        m_prevKeyUp = true;

        input_t iKey = K_MAKE_COORD_CODE(K_MOUSEKUP, x, y);
        PutInput(iKey);
        return;
    }

    m_fMouseTrack = true;
    m_prevKeyUp = false;

    input_t iKeyMode = 0;
    if(pMouseEvent->dwControlKeyState & SHIFT_PRESSED)
        iKeyMode |= K_SHIFT;
    if(pMouseEvent->dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
        iKeyMode |= K_CTRL;
    if(pMouseEvent->dwControlKeyState & (LEFT_ALT_PRESSED  | RIGHT_ALT_PRESSED))
        iKeyMode |= K_ALT;

    input_t iMKey = 0;
    switch(pMouseEvent->dwButtonState)
    {
    case FROM_LEFT_1ST_BUTTON_PRESSED:
        iMKey = K_MOUSEKL;
        break;
    case RIGHTMOST_BUTTON_PRESSED:
        iMKey = K_MOUSEKR;
        break;
    case FROM_LEFT_2ND_BUTTON_PRESSED:
        iMKey = K_MOUSEKM;
        break;
    }

    input_t iMType = 0;
    switch(pMouseEvent->dwEventFlags)
    {
#ifdef MOUSE_WHEELED
    case MOUSE_WHEELED:
        {
            iMType = K_MOUSEW;
            DWORD st = pMouseEvent->dwButtonState & 0xffff0000;
            if (st != 0)
            {
                if ((st & 0x80000000) != 0)
                    iMKey = K_MOUSEWDN | K_MOUSEW;
                else
                    iMKey = K_MOUSEWUP | K_MOUSEW;
                m_fMouseTrack = false;
            }
        }
        break;
#endif
    default:
        {
            clock_t t = clock();
            //LOG(DEBUG) << "dt=" << t - m_prevTime;
            if(iMKey)
            {
                if((m_prevKey & K_TYPEMASK) == iMKey && m_prevX == x && m_prevY == y && m_prevTime + CLOCKS_PER_SEC / 2 > t)
                {
                    if(prevUp)
                        switch(m_prevKey & K_MODMASK)
                        {
                        case 0:
                            //LOG(DEBUG) << "double click";
                            iMType = K_MOUSE2;
                            break;
                        case K_MOUSE2:
                            //LOG(DEBUG) << "triple click";
                            iMType = K_MOUSE3;
                            break;
                        default:
                            //LOG(DEBUG) << "simple click";
                            break;
                        }
                }

                if(!prevUp && !iMType && m_prevX == x && m_prevY == y && (m_prevKey & K_TYPEMASK) == iMKey)
                    //same event
                    return;

                m_prevX = x;
                m_prevY = y;
                m_prevKey = iMKey | iMType;
                m_prevTime = t;
            }
        }
    }

    input_t iKey = K_MAKE_COORD_CODE(K_MOUSE | iMKey | iMType | iKeyMode, x, y);
    PutInput(iKey);
}

//////////////////////////////////////////////////////////////////////////////
void InputWin32::ProcessResizeEvent(WINDOW_BUFFER_SIZE_RECORD* pWindowBufferSizeEvent)
{
    LOG(DEBUG) << "ResizeEvent x=" << pWindowBufferSizeEvent->dwSize.X << " y=" << pWindowBufferSizeEvent->dwSize.Y;

    pos_t x = pWindowBufferSizeEvent->dwSize.X;
    pos_t y = pWindowBufferSizeEvent->dwSize.Y;
    if (x == MAX_COORD && y == MAX_COORD)
        //special case for screen size calculating
        return;
    
    auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO sbInfo;
    bool rc = GetConsoleScreenBufferInfo(handle, &sbInfo);
    if (rc)
    {
        if (x != sbInfo.dwSize.X && y != sbInfo.dwSize.Y)
        {
            LOG(WARNING) << "Scr size=" << sbInfo.dwSize.X << "/" << sbInfo.dwSize.Y;
            return;
        }
    }

    if (x > MAX_COORD) //???
        x = MAX_COORD;
    if (y > MAX_COORD)
        y = MAX_COORD;

    input_t iKey = K_MAKE_COORD_CODE(K_RESIZE, x, y);
    PutInput(iKey);
}

//////////////////////////////////////////////////////////////////////////////
void InputWin32::ProcessFocusEvent(FOCUS_EVENT_RECORD* pFocusEvent)
{
    //LOG(DEBUG) << "FocusEvent set=" << pFocusEvent->bSetFocus;
    
    input_t iKey;
    if(pFocusEvent->bSetFocus)
        iKey = K_FOCUSSET;
    else
        iKey = K_FOCUSLOST;

    PutInput(iKey);
}

//////////////////////////////////////////////////////////////////////////////
void InputWin32::ProcessMenuEvent([[maybe_unused]]MENU_EVENT_RECORD* pMenuEvent)
{
    //LOG(DEBUG) << "MenuEvent id=" << pMenuEvent->dwCommandId;
}

} //namespace _Console

#endif //WIN32
