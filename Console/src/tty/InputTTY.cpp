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
#ifndef WIN32

#include "Console/tty/InputTTY.h"
#include "Console/tty/KeyMap.h"
#include "Console/tty/TermcapMap.h"
#include "Console/tty/Mouse.h"
#include "utils/logger.h"
#include "utfcpp/utf8.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <term.h>
#include <signal.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#ifdef _POSIX_VDISABLE
    #define NULL_VALUE _POSIX_VDISABLE
#else
    #define NULL_VALUE 255
#endif

namespace _Console
{

std::atomic_bool ConsoleInput::s_fExit {false};
std::atomic_bool InputTTY::s_fResize {false};
std::atomic_bool InputTTY::s_fCtrlC {false};


//////////////////////////////////////////////////////////////////////////////
bool InputTTY::LoadKeyCode()
{
    LOG(DEBUG) << "LoadKeyCode";

    //load predefined keys first
    //for solveing XTERM key mapping error
    for(const auto& map : g_keyMap)
        m_KeyMap.AddKey(map.sequence, map.code);

#ifdef __linux__
    if(!strcmp(getenv("TERM"), "linux"))
    {
        LOG(DEBUG) << "keyMap default linux";
        for(const auto& map : g_keyMap1)
            m_KeyMap.AddKey(map.sequence, map.code);
    }
    else
#endif
    {
        LOG(DEBUG) << "keyMap 2";
        for(const auto& map : g_keyMap2)
            m_KeyMap.AddKey(map.sequence, map.code);
    }

    LOG(DEBUG) << "keyCap";
    for(const auto& cap: g_keyCap)
    {
        char buff[16];
        char* pbuff = buff;
        char* str = tgetstr(const_cast<char*>(cap.id), &pbuff);
        if(str)
            m_KeyMap.AddKey(str, cap.code);
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////
std::string InputTTY::GetConsoleCP()
{
    auto locale = setlocale(LC_CTYPE, "");
    std::string lc_type = locale != nullptr ? locale : "";
    LOG(DEBUG) << "LC_CTYPE=" << lc_type;
    if(lc_type.empty())
        return lc_type;

    auto pos = lc_type.find(".");
    if(pos == std::string::npos)
        return lc_type;
    else
        return lc_type.substr(pos + 1);
}


//////////////////////////////////////////////////////////////////////////////
bool InputTTY::Init() 
{
    int rc = 0; 

    if(m_stdin > 0)
        return false;

    InitMouse();

    m_stdin = open("/dev/tty", O_RDWR);
    if(m_stdin <= 0)
    {
        LOG(WARNING) << "'/dev/tty' not found";
        m_stdin = fileno(stdin);
        if(m_stdin < 0)
        {
            LOG(ERROR) << "file 'stdin' not exists";
            throw std::runtime_error {"file 'stdin' not exists"};
        }
    }
    //LOG(DEBUG) << "stdin no=" << fileno(stdin) << " stdin tty=" << ttyname(fileno(stdin));
    LOG(DEBUG) << "stdin file=" << m_stdin << " open tty=" << ttyname(m_stdin);

#ifdef __linux__
    //Reads the shift state of the keyboard by using
    //a semi-documented ioctl() call the Linux kernel.
    int nArg = 6; /* TIOCLINUX function #6 */
    rc = ioctl(m_stdin, TIOCLINUX, &nArg);
    if(0 == rc)
    {
        m_fTiocLinux = 1;
        LOG(DEBUG) << "TIOCLINUX present";
    }
#endif

    rc = tcgetattr(m_stdin, &m_termold);
    if(-1 == rc)
        return false;

    memcpy(&m_termnew, &m_termold, sizeof(m_termnew));
    m_termnew.c_iflag     &= ~(ECHO | INLCR | ICRNL);
    m_termnew.c_lflag      = ISIG | NOFLSH;

    m_termnew.c_cc[VMIN]   = 1;
    m_termnew.c_cc[VTIME]  = 0;

    m_termnew.c_cc[VQUIT]  = NULL_VALUE;  //to ignore '^\'
    m_termnew.c_cc[VSTART] = NULL_VALUE;  //to ignore '^Q' XON
    m_termnew.c_cc[VSTOP]  = NULL_VALUE;  //to ignore '^S' XOFF
    m_termnew.c_cc[VSUSP]  = NULL_VALUE;  //to ignore '^Z'
#ifdef VLNEXT
    m_termnew.c_cc[VLNEXT] = NULL_VALUE;  //to ignore '^V'
#endif

    rc = tcsetattr(m_stdin, TCSANOW, &m_termnew);
    if(-1 == rc)
        return false;

    m_fTerm = true;

    rc = LoadKeyCode();

    InitSignals();

    auto cp = GetConsoleCP();
    LOG(DEBUG) << "Console input inited, CP=" << cp;

    return true;
}


//////////////////////////////////////////////////////////////////////////////
void InputTTY::Deinit()
{
    if(m_stdin < 0)
        return;

    if(m_fTerm)
    {
        tcsetattr(m_stdin, TCSANOW, &m_termold);
        m_fTerm = 0;
    }

    while((-1 == close(m_stdin)) && (errno == EINTR));
    m_stdin = -1;

    DeinitMouse();

    LOG(DEBUG) << "Deinited";
}


//////////////////////////////////////////////////////////////////////////////
void InputTTY::Abort(int signal)
{
    LOG(DEBUG) << "MyAbort " << signal;
    sleep(1);
    _exit(signal);
}


//////////////////////////////////////////////////////////////////////////////
void InputTTY::Resize([[maybe_unused]] int signal)
{
    //LOG(DEBUG) << "Resize";
    s_fResize = 1;
}


//////////////////////////////////////////////////////////////////////////////
void InputTTY::CtrlC([[maybe_unused]] int signal)
{
    //LOG(DEBUG) << "CtrlC";
    s_fCtrlC = 1;
}


//////////////////////////////////////////////////////////////////////////////
void InputTTY::Child([[maybe_unused]] int signal)
{
    LOG(DEBUG) << "Child";
    pid_t pid;
    int   stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    {
        LOG(DEBUG) << "Process " << pid << " exited with status: " << stat;
    }
}


//////////////////////////////////////////////////////////////////////////////
bool InputTTY::InitSignals()
{
    //Catch signals

    int iMin = 1;
    int iMax = NSIG;

    for(int i = iMin; i < iMax; ++i)
    {
        if(i == SIGCHLD)
            signal(i, Child);
        else if(i == SIGPIPE || i == SIGCONT)
            signal(i, SIG_IGN);
        else if(i == SIGWINCH)
            signal(i, Resize);
        else if(i == SIGINT)
            signal(i, CtrlC);
        else
            signal(i, Abort);
    }

  return true;
}


//////////////////////////////////////////////////////////////////////////////
bool InputTTY::InputPending(const std::chrono::milliseconds& WaitTime)
{
    if(m_stdin < 0)
    {
        errno = EBADF;
        return false;
    }

    if(GetInputLen())
        return true;

    long secs  = 0;
    long usecs = WaitTime.count() * 1000;

    struct timeval wait;
    wait.tv_sec = secs;
    wait.tv_usec = usecs;

    fd_set Read_FD_Set;
    FD_ZERO(&Read_FD_Set);
    FD_SET(m_stdin, &Read_FD_Set);

    int sel = m_stdin;
    int mouse_fd = GetMouseFD();

    if(mouse_fd > 0)
    {
        FD_SET(mouse_fd, &Read_FD_Set);
        if(mouse_fd > sel)
            sel = mouse_fd;
    }

    int rc = select(sel + 1, &Read_FD_Set, NULL, NULL, &wait);
    if(rc > 0)
    {
        if(FD_ISSET(m_stdin, &Read_FD_Set))
            ProcessInput(false);
        else
            ProcessInput(true);
    }
    else if(rc == 0)
        //timeout
        ProcessSignals();
    else
        ProcessSignals();

    return 0 != GetInputLen();
}


//////////////////////////////////////////////////////////////////////////////
size_t InputTTY::ReadConsole(std::string& str, size_t n)
{
    struct timeval wait;
    wait.tv_sec  = 0;
    wait.tv_usec = 100000;// 1/10 sec

    fd_set Read_FD_Set;
    FD_ZERO(&Read_FD_Set);
    FD_SET(m_stdin, &Read_FD_Set);

    int s = select(m_stdin + 1, &Read_FD_Set, NULL, NULL, &wait);
    if(s > 0)
    {
        std::string buff(MaxInputLen, 0);

        int rc = read(m_stdin, buff.data(), n);
        if(rc > 0)
        {
            str.append(std::string_view(buff).substr(0, rc));
            return static_cast<size_t>(rc);
        }
    }

    return 0;
}


input_t InputTTY::ProcessMouse(pos_t x, pos_t y, input_t k)
{
    input_t iMType = 0;
    bool prevUp = m_prevUp;

    //LOG(DEBUG) << __FUNC__ << " prevup=" << prevUp;
    if(k == K_MOUSEKUP)
        m_prevUp = true;
    else
    {
        m_prevUp = false;

        const std::chrono::milliseconds waitTicks {500ms};
        auto t = std::chrono::steady_clock::now();
        auto dtms = std::chrono::duration_cast<std::chrono::milliseconds>(t - m_prevTime);
        //LOG(DEBUG) << "dt=" << dtms.count() << " c=" << waitTicks.count();

        if((m_prevKey & K_TYPEMASK) == k && m_prevX == x && m_prevY == y && dtms < waitTicks)
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

        m_prevX = x;
        m_prevY = y;
        m_prevKey = k | iMType;
        m_prevTime = t;
    }

    return iMType;
}

//////////////////////////////////////////////////////////////////////////////
void InputTTY::ProcessInput(bool fMouse)
{
    std::string buff;

    size_t iLen = 0;
    input_t iKey = K_ERROR;
    input_t iKeyMode = 0;
    unsigned char c = 0;
    int rc = 0;

#ifdef __linux__
    if(m_fTiocLinux)
    {
        //Reads the shift state of the keyboard by using
        //a semi-documented ioctl() call the Linux kernel.
        int nArg = 6; /* TIOCLINUX function #6 */
        int rc  = ioctl(m_stdin, TIOCLINUX, &nArg);
        if(!rc && nArg)
        {
            //LOG(DEBUG) << "keymode=" << nArg;

            if(nArg & 1)
                iKeyMode = K_SHIFT;
            if(nArg & 4)
                iKeyMode = K_CTRL;
        }

        if((m_prevMode & K_SHIFT) && !(iKeyMode & K_SHIFT))
            PutInput(K_RELEASE | K_SHIFT);

        m_prevMode = iKeyMode;
    }
#endif

    if(fMouse)
    {
        iKey = ReadMouse();
        if(iKey && iKey != K_ERROR)
        {
            pos_t x = K_GET_X(iKey);
            pos_t y = K_GET_Y(iKey);
            input_t k = iKey & K_TYPEMASK;

            input_t iMType = ProcessMouse(x, y, k);
            //LOG(DEBUG) << "Mouse input iKey=" << std::hex << (iKey | iMType | iKeyMode) << std::dec;
            PutInput(iKey | iMType | iKeyMode);
        }
        return;
    }

    rc = ReadConsole(buff, 1);
    if(rc == 1)
    {
        ++iLen;
        if(buff[0] == 0x1b)
        {
            //read ESC sequence
            rc = ReadConsole(buff, MaxInputLen);
            if(rc > 0)
                iLen += rc;
        }
        else if((buff[0] & 0x80) != 0)
        {
            //utf8
            rc = ReadConsole(buff, MaxInputLen);
            if(rc > 0)
                iLen += rc;

            //LOG(DEBUG) << "utf8=" << KeyMapper::CastString(buff);
            std::u16string wstr = utf8::utf8to16(buff);
            for(auto& wc : wstr)
                PutInput(wc | iKeyMode);
            return;
        }
    }

#ifdef USE_MOUSE
#ifdef OLD_MOUSE
    if(iLen >= 6 && buff[0] == 0x1b && buff[1] == 0x5b && buff[2] == 0x4d)//"\x1b[M"
    {
        //old mouse input
        pos_t x = buff[4] - 0x21;
        pos_t y = buff[5] - 0x21;

        input_t k = buff[3] & 0x63;
        switch(k)
        {
        case '@':
        case 0x20: k = K_MOUSEKL;  break;
        case 'A':
        case 0x21: k = K_MOUSEKM;  break;
        case 'B':
        case 0x22: k = K_MOUSEKR;  break;
        case 0x23: k = K_MOUSEKUP; break;

        case 0x60: k = K_MOUSEWUP | K_MOUSEW; break;
        case 0x61: k = K_MOUSEWDN | K_MOUSEW; break;

        default:   k = K_ERROR; break;
        }

        input_t iMType = 0;

        if((k & K_MOUSEW) == 0)
            iMType = ProcessMouse(x, y, k);

        iKey = K_MAKE_COORD_CODE(k | iMType, x, y);

        //LOG(DEBUG) << "Mouse input iKey=" << std::hex << iKey << std::dec;
    }
#else
    if(iLen >= 6 && buff[0] == 0x1b && buff[1] == 0x5b && buff[2] == 0x3c)//"\x1b[<"
    {
        //new mouse input
        int k, x, y;
        char m;
        int n = sscanf(buff.c_str(), "\x1b[<%d;%d;%d%c", &k, &x, &y, &m);
        if(n == 4)
        {
            --x;
            --y;
            //LOG(DEBUG) << "Mouse input k=0x" << std::hex << k << std::dec << " m=" << m << " x=" << x << " y=" << y;

            input_t key{K_ERROR};
            input_t iMType{};

            if(m == 'm')
                key = K_MOUSEKUP;
            else
            {
                if(k & 0x40)
                {
                    //wheel
                    if(k == 0x40)
                        key = K_MOUSEWUP | K_MOUSEW;
                    if(k == 0x41 )
                        key = K_MOUSEWDN | K_MOUSEW;
                }
                else
                {
                    switch(k & 0x3)
                    {
                    case 0:  key = K_MOUSEKL; break;
                    case 1:  key = K_MOUSEKM; break;
                    case 2:
                    case 3:  key = K_MOUSEKR; break;
                    }
                    if(k & 0x10)
                        iKeyMode |= K_CTRL;
                    if(k & 0x8)
                        iKeyMode |= K_ALT;
                    if(k & 0x4)
                        iKeyMode |= K_SHIFT;
                }
            }

            if((k & K_MOUSEW) == 0)
                iMType = ProcessMouse(x, y, key);

            iKey = K_MAKE_COORD_CODE(key | iKeyMode | iMType, x, y);

            //LOG(DEBUG) << "Mouse input iKey=" << std::hex << iKey << std::dec;
        }
    }
#endif //!OLD_MOUSE
    else
#endif //USE_MOUSE

    if(iLen >= 2 && buff[0] == 0x1b && buff[1] == 0x1b)
    {
        if(iLen == 2)
            //it is ALT+ESC
            iKey = K_ESC | K_ALT;
        else
            //try as ATL+...
            iKey = m_KeyMap.GetCode(buff.substr(1)) | K_ALT;
    }
    else if(iLen)
    {
        if(iLen == 1 && buff[0] == 0x1b)
            iKey = 0x1b;
        else
            iKey = m_KeyMap.GetCode(buff);

        auto CheckKeys = [&buff, &iLen, &iKey, &iKeyMode]()
        {
            if(iKey != 0 && iKey != K_ERROR)
                return;

            if (iLen != 6 || std::string_view(buff).substr(0, 4) != "\x1b[1;")
                return;

            auto k = buff[4];
            if (k >= '2' && k <= '8')
            {
                k -= '1';
                if (k & 0x1)
                    iKeyMode |= K_SHIFT;
                if (k & 0x2)
                    iKeyMode |= K_ALT;
                if (k & 0x4)
                    iKeyMode |= K_CTRL;
            }

            switch (buff[5])
            {
            case 'A':
                iKey = K_UP;
                break;
            case 'B':
                iKey = K_DOWN;
                break;
            case 'C':
                iKey = K_RIGHT;
                break;
            case 'D':
                iKey = K_LEFT;
                break;
            case 'H':
                iKey = K_HOME;
                break;
            case 'F':
                iKey = K_END;
                break;
            }
        };

        CheckKeys();

        int t = 10;
        while(iKey == 0 && iLen >= 2 && buff[0] == 0x1b && buff[1] != 0x1b && t--)
        {
            //try to read any more
            LOG(DEBUG) << "read more";
            rc = ReadConsole(buff, MaxInputLen);
            if(rc > 0)
            {
                iLen += rc;
                iKey = m_KeyMap.GetCode(buff);
                CheckKeys();
            }
        }

        if(iKey == K_ERROR)
        {
            if(iLen == 1)
            {
                c = buff[0];
                iKey = K_CHAR;
            }
            else if(iLen == 2 && buff[0] == 0x1b)
            {
                //try as ATL+...
                iKeyMode = K_ALT;
                iKey = m_KeyMap.GetCode(buff.substr(1)) | K_ALT;
                if(iKey == K_ERROR)
                {
                    if(buff[1] >= 'a' && buff[1] <= 'z')
                        c = buff[1] - 0x20;
                    else
                        c = buff[1];
                    iKey = K_CHAR;
                }
            }
        }

        if(iKey == K_CHAR)
        {
            if(c == K_TAB || c == K_ENTER || c == K_ESC)
            {
                //special symbols
                iKey = c | iKeyMode;
            }
            else if(c < ' ')
            {
                //not printed symbols
                if(!c)
                    iKey = ' ' | K_CTRL | iKeyMode;
                else
                    iKey = ('A' - 1 + c) | K_CTRL | iKeyMode;
            }
            else
            {
                //common symbols
                if(c <= 0x7f)
                    //ascii
                    iKey = c | iKeyMode;
                else
                {
                    //utf8 ???
                    iKey = K_ERROR;
                }
            }
        }
    }

    if(iKey && iKey != K_ERROR)
    {
        PutInput(iKey | iKeyMode);
    }
    else
    {
        LOG(WARNING) << "Not found seq=" << CastEscString(buff);
    }
}


//////////////////////////////////////////////////////////////////////////////
void InputTTY::ProcessSignals()
{
    if(s_fResize)
    {
        //LOG(DEBUG) << "ProcessSignal: Resize";
        s_fResize = 0;

        m_termcap.LoadTermcap();

        pos_t x = 0;
        pos_t y = 0;
        //read screen size
        //resize screen
        if(m_ResizeCallback)
            m_ResizeCallback(x, y);

        PutInput(K_MAKE_COORD_CODE(K_RESIZE, x, y));
    }

    if(s_fCtrlC)
    {
        //LOG(DEBUG) << "ProcessSignal: CtrlC";
        s_fCtrlC = 0;

        PutInput('C' | K_CTRL);
    }
}

} //namespace _Console

#endif //!WIN32
