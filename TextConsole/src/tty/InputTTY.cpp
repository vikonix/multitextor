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
#ifndef WIN32

#include "tty/InputTTY.h"
#include "tty/KeyMap.h"
#include "tty/TermcapMap.h"
#include "logger.h"

#define USE_MOUSE
#include "tty/Mouse.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <term.h>
#include <signal.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <codecvt>

#ifdef _POSIX_VDISABLE
    #define NULL_VALUE _POSIX_VDISABLE
#else
    #define NULL_VALUE 255
#endif


std::atomic_bool InputTTY::s_fResize {false};
std::atomic_bool InputTTY::s_fCtrlC {false};
std::atomic_bool InputTTY::s_fExit {false};


//////////////////////////////////////////////////////////////////////////////
bool InputTTY::LoadKeyCode()
{
    LOG(DEBUG) << "LoadKeyCode";

    //load predefined keys first
    //for solveing XTERM key mapping error
    size_t i;
    for(i = 0; g_keyMap[i].code; ++i)
        m_KeyMap.AddKey(g_keyMap[i].sequence, g_keyMap[i].code);
    
#ifdef __linux__
    if(!strcmp(getenv("TERM"), "linux"))
    {
        LOG(DEBUG) << "keyMap default linux";
    for(i = 0; g_keyMap1[i].code; ++i)
        m_KeyMap.AddKey(g_keyMap1[i].sequence, g_keyMap1[i].code);
    }
    else
#endif
    {
        LOG(DEBUG) << "keyMap 2";
        for(i = 0; g_keyMap2[i].code; ++i)
            m_KeyMap.AddKey(g_keyMap2[i].sequence, g_keyMap2[i].code);
    }

    LOG(DEBUG) << "keyCap";
    for(i = 0; g_keyCap[i].id; ++i)
    {
        char buff[16];
        char* pbuff = buff;
        char* str = tgetstr(g_keyCap[i].id, &pbuff);
        if(str)
            m_KeyMap.AddKey(str, g_keyCap[i].code);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
bool InputTTY::LoadTermcap()
{
    char* term;
    if(NULL == (term = getenv("TERM")))
    {
        return false;
    }

    if(1 != tgetent(m_termcapBuff, term))
    {
        return false;
    }

    LOG(DEBUG) << "LoadTermcap term=" << term;
    return true;
}


//////////////////////////////////////////////////////////////////////////////
std::string InputTTY::GetConsoleCP()
{
  std::string lc_type = setlocale(LC_CTYPE, "");
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
        return false;

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

    rc = LoadTermcap();
    rc = LoadKeyCode();

    InitSignals();

    auto cp = GetConsoleCP();
    LOG(DEBUG) << "Console CP=" << cp;
    LOG(DEBUG) << "Inited";

    return true;
}


//////////////////////////////////////////////////////////////////////////////
void InputTTY::Deinit()
{
    //DeinitMouse();
    if(m_stdin < 0)
        return;
    
    if(m_fTerm)
    {
        tcsetattr(m_stdin, TCSANOW, &m_termold);
        m_fTerm = 0;
    }

    while((-1 == close(m_stdin)) && (errno == EINTR));
    m_stdin = -1;

    LOG(DEBUG) << "Deinited";
}

bool InputTTY::SwitchToStdConsole()
{
    //DeinitMouse();
    tcsetattr(m_stdin, TCSANOW, &m_termold);

    return true;
}


bool InputTTY::RestoreConsole()
{
    //InitMouse();
    tcsetattr(m_stdin, TCSANOW, &m_termnew);

    return true;
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
            str.insert(str.size(), buff, 0, rc);
            return static_cast<size_t>(rc);
        }
    }
    
    return 0;
}


input_t InputTTY::ProcessMouse(pos_t x, pos_t y, input_t k)
{
    input_t iMType = 0;
    bool prevUp = m_prevUp;
    
    if(k == K_MOUSEKUP)
        m_prevUp = true;
    else
    {
        m_prevUp = false;
        
        const clock_t waitTicks = 500;
        clock_t t = clock();
        //LOG(DEBUG) << "dt=" << std::dec << t - m_prevTime << " c=" << waitTicks;

        if((m_prevKey & K_TYPEMASK) == k && m_prevX == x && m_prevY == y && m_prevTime + waitTicks > t)
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
            
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wstr = converter.from_bytes(buff);
            for(auto& wc : wstr)
                PutInput(wc | iKeyMode);
            return;
        }
    }

#ifdef USE_MOUSE
    if(iLen >= 6 && buff[0] == 0x1b && buff[1] == 0x5b && buff[2] == 0x4d)
    {
        //mouse input
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

        //LOG(DEBUG) << "Mouse input iKey=" << std::hex << iKey;
    }
    else
#endif

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
        LOG(WARNING) << "Not found seq=" << KeyMapper::CastString(buff);
    }
}


//////////////////////////////////////////////////////////////////////////////
void InputTTY::ProcessSignals()
{
    if(s_fResize)
    {
        //LOG(DEBUG) << "ProcessSignal: Resize";
        s_fResize = 0;

        LoadTermcap();

        pos_t x = 0;
        pos_t y = 0;
        //read screen size
        //resise screen

        PutInput(K_MAKE_COORD_CODE(K_RESIZE, x, y));
    }

    if(s_fCtrlC)
    {
        //LOG(DEBUG) << "ProcessSignal: CtrlC";
        s_fCtrlC = 0;

        PutInput('C' | K_CTRL);
    }
}

#endif //!WIN32
