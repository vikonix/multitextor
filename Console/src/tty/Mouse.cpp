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
#ifndef WIN32

#ifdef __linux__
  #define USE_GPM
#endif

#include "tty/Mouse.h"
#include "KeyCodes.h"

#ifndef HAVE_MAIN
  #include "utils/logger.h"
#else
  #include <iostream>    
  #define LOG(t) std::cout
#endif

#include <dlfcn.h>
#include <string.h>
#include <errno.h>

#ifdef USE_GPM
  #include <gpm.h>
#endif

/////////////////////////////////////////////////////////////////////////////
static uint32_t s_fMouseOpen = 0;


/////////////////////////////////////////////////////////////////////////////
#ifdef USE_GPM
static void* hLib = NULL;

typedef int (*p_gpm_open)(Gpm_Connect *, int);
typedef int (*p_gpm_close)();
typedef int (*p_gpm_getevent)(Gpm_Event *);

static p_gpm_open       gpm_open             = NULL;
static p_gpm_close      gpm_close            = NULL;
static p_gpm_getevent   gpm_getevent         = NULL;

static int*             p_gpm_fd             = NULL;
static int*             p_gpm_visiblepointer = NULL;
#endif //USE_GPM


/////////////////////////////////////////////////////////////////////////////
//xwindows mouse only
bool x_InitMouse()
{
    LOG(DEBUG) << "x_InitMouse";

    if(s_fMouseOpen)
        return true;

    char* term;
    if(NULL == (term = getenv("TERM")) || (strncmp(term, "xterm", 5) && strncmp(term, "iterm", 5)))
        //not xterm
        return false;

    s_fMouseOpen = 1;
    LOG(DEBUG) << "xterm mouse";
    // save old highlight tracking
    printf("\x1b[?1001s");
    fflush(stdout);
    
    // enable mouse tracking
    //key down, tracking and up
    printf("\x1b[?1002h");
    fflush(stdout);

#ifndef OLD_MOUSE    
    // enable SGR extended mouse reporting
    printf ("\x1b[?1006h");
    fflush(stdout);
#endif
    return true;
}

bool x_DeinitMouse()
{
    if(s_fMouseOpen != 1)
        return true;

    LOG(DEBUG) << "x_DeinitMouse";

    s_fMouseOpen = 0;

#ifndef OLD_MOUSE
    // disable SGR extended mouse reporting
    printf ("\x1b[?1006l");
    fflush(stdout);
#endif

    // disable mouse tracking
    printf("\x1b[?1002l");
    fflush(stdout);
    
    // restore old highlight tracking
    printf("\x1b[?1001r");
    fflush(stdout);

    return true;
}


/////////////////////////////////////////////////////////////////////////////
#ifdef USE_GPM
//gpm lib mouse
bool LoadGpm()
{
    hLib = dlopen("libgpm.so", RTLD_LAZY);
    if(!hLib)
        hLib = dlopen("libgpm.so.1", RTLD_LAZY);

    if(!hLib)
    {
        LOG(ERROR) << "LoadGpm ERROR " << dlerror();
        return false;
    }

    LOG(DEBUG) << "LoadGpm OK";
    gpm_open              = (p_gpm_open)        dlsym(hLib, "Gpm_Open");
    gpm_close             = (p_gpm_close)       dlsym(hLib, "Gpm_Close");
    gpm_getevent          = (p_gpm_getevent)    dlsym(hLib, "Gpm_GetEvent");

    p_gpm_fd              = (int *)             dlsym(hLib, "gpm_fd");
    p_gpm_visiblepointer  = (int *)             dlsym(hLib, "gpm_visiblepointer");

    return true;
}


bool CloseGpm()
{
    if(!hLib)
        return true;

    LOG(DEBUG) << "CloseGpm";

    dlclose(hLib);
    hLib      = NULL;
    gpm_open  = NULL;
    gpm_close = NULL;
    return true;
}


bool g_InitMouse()
{
    if(s_fMouseOpen)
        return true;

    if(!gpm_open)
        LoadGpm();
    if(!gpm_open)
        return false;

    Gpm_Connect conn;

    conn.eventMask   = GPM_MOVE|GPM_DRAG|GPM_DOWN|GPM_UP; // Want to know about events
    conn.defaultMask = GPM_MOVE|GPM_HARD;                 // handle by default
    conn.minMod      =  0;                                // want everything
    conn.maxMod      = ~0;                                // all modifiers included

    int rc = (*gpm_open)(&conn, 0);

    if(rc != -1)
    {
        s_fMouseOpen = 2;
        if(p_gpm_fd)
            LOG(DEBUG) << "gpm_fd=" << *p_gpm_fd;
        if(p_gpm_visiblepointer)
            *p_gpm_visiblepointer = 1;
    }
    else
    {
        LOG(DEBUG) << "Connect to gpm mouse server rc=" << rc << " errno=" << errno;
    }

    return *p_gpm_fd > 0;
}


bool g_DeinitMouse()
{
    if(s_fMouseOpen == 2 && gpm_close)
        (*gpm_close)();

    s_fMouseOpen = 0;
    CloseGpm();

    return true;
}
#endif //USE_GPM

/////////////////////////////////////////////////////////////////////////////
bool _InitMouse()
{
    x_InitMouse();
#ifdef USE_GPM
    g_InitMouse();
#endif
    return s_fMouseOpen != 0;
}


bool _DeinitMouse()
{
    x_DeinitMouse();
#ifdef USE_GPM
    g_DeinitMouse();
#endif
    return true;
}


int _GetMouseFD()
{
#ifdef USE_GPM
    if(s_fMouseOpen == 2)
        return *p_gpm_fd;
    else
#endif
        return -1;
}


input_t _ReadMouse()
{
#ifdef USE_GPM
    if(s_fMouseOpen == 2)
    {
        Gpm_Event gpmEvent;

        int rc = (*gpm_getevent)(&gpmEvent);
        if(rc == 1)
        {
            int x = gpmEvent.x - 1;
            int y = gpmEvent.y - 1;
            input_t k = 0;

            if(x < 0)
                x = 0;
            if(y < 0)
                y = 0;

            if(gpmEvent.type & GPM_UP)
                k = K_MOUSEKUP;
            else if(gpmEvent.buttons & GPM_B_LEFT)
                k = K_MOUSEKL;
            else if(gpmEvent.buttons & GPM_B_RIGHT)
                k = K_MOUSEKR;
            else if(gpmEvent.buttons & GPM_B_MIDDLE)
                k = K_MOUSEKM;
/*
            LOG(DEBUG) << "ReadMouse x=" << x << " y=" << y 
                << " b=" << gpmEvent.buttons << " t=" << gpmEvent.type << " k=" << k 
                << " wx=" << gpmEvent.wdx <<  " wy=" << gpmEvent.wdy;
//*/                
            if(k == 0 && gpmEvent.wdy != 0)    
            {
                if(gpmEvent.wdy == 1)
                    k = K_MOUSEWUP;
                else if(gpmEvent.wdy == -1)
                    k = K_MOUSEWDN;
            }
            if(k)
                return k | (x << 8) | y;
        }
    }
#endif
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
#ifdef HAVE_MAIN
  #include <stdio.h>
  #include <stdlib.h>
  
#ifdef USE_GPM
  #include <linux/tiocl.h>
#endif

int main()
{
    int c;

    printf("GPM mouse test\n\n");

    _InitMouse();

#ifndef USE_GPM
    while((c = getchar()) != EOF)
        printf("%x ", c);

#else
    if(p_gpm_fd)
    {
        printf("gpm_fd=%d\n", *p_gpm_fd);
        if(*p_gpm_fd != -1)
        {
            Gpm_Event gpmEvent;
            int rc = 1;

            while(rc)
            {
                rc = (*gpm_getevent)(&gpmEvent);

                printf("event rc=%d x=%d y=%d type=%x buttons=%x\n", rc, gpmEvent.x, gpmEvent.y, gpmEvent.type, gpmEvent.buttons);
                if(gpmEvent.type & GPM_MOVE)
                    printf("move\n");
                if(gpmEvent.type & GPM_DOWN)
                    printf("down\n");
                if(gpmEvent.type & GPM_UP)
                    printf("up\n");
                if(gpmEvent.type & GPM_DRAG)
                    printf("drag\n");

                struct _showcmd 
                {
                    char  cmd;
                    short xs, ys, xe, ye;
                    short type;
                } showcmd;

                showcmd.cmd  = TIOCL_SETSEL;
                showcmd.xs   = gpmEvent.x;
                showcmd.xe   = gpmEvent.x;
                showcmd.ys   = gpmEvent.y;
                showcmd.ye   = gpmEvent.y;
                showcmd.type = TIOCL_SELPOINTER;
                ioctl(*p_gpm_fd, TIOCLINUX, &showcmd);
            }
        }
    }
#endif //USE_GPM

    _DeinitMouse();

    printf("OK\n");
    return 0;
}
#endif //HAVE_MAIN

#endif //!WIN32
