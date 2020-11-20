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
//#include <vld.h>

#include "logger.h"
#include "win32/InputWin32.h"
#include "tty/InputTTY.h"

#include "win32/ScreenWin32.h"
#include "tty/ScreenTTY.h"

#include <iostream>
#include <chrono>


void InputTest()
{
    std::cout << "ConsoleInput test" << std::endl;
    LOG(INFO);
    LOG(INFO) << "ConsoleInput test";

#ifdef WIN32
    InputWin32 console;
#else
    InputTTY console;
#endif
    console.Init();

    while (1)
    {
        using namespace std::chrono_literals;
        console.InputPending(500ms);
        input_t key = console.GetInput();

        LOG_IF(key, INFO) << "  " << ConsoleInput::CastKeyCode(key);
        if (key == K_EXIT || key == K_ALT + 'X')
            break;
    }

    console.Deinit();
    LOG(INFO) << "ConsoleInput test end";
}

void OutputTest()
{
    std::cout << "ConsoleOutput test" << std::endl;
    LOG(INFO);
    LOG(INFO) << "ConsoleOutput test";

#ifdef WIN32
    InputWin32 console;
    ScreenWin32 screen;
#else
    InputTTY console;
    ScreenTTY screen;
#endif

    console.Init();
    screen.Init();

    auto waitKey = [&console, &screen]() {
        screen.Flush();
        while (1)
        {
            using namespace std::chrono_literals;
            console.InputPending();
            input_t key = console.GetInput();
            LOG_IF(key, INFO) << "  " << ConsoleInput::CastKeyCode(key);
            if (key == K_SPACE)
                break;
        }
    };

    screen.WriteConsoleTitle(L"Console Screen \x428");
    screen.ClrScr();
    screen.SetCursor(cursor_t::CURSOR_NORMAL);
    screen.GotoXY(0, 0);
    waitKey();

    screen.SetTextAttr(TEXT_BLUE);
    screen.WriteChar(u'\x428'); // russian sh:'Ш'

    screen.SetTextAttr(TEXT_RED);
    screen.WriteChar('[');
    for(char16_t i = 1; i < ACS_MAX; ++i)
        screen.WriteChar(i);
    screen.WriteChar(']');

    screen.SetTextAttr(COLOR_INVERSE(TEXT_GREEN));
    screen.WriteStr(u"Text\x428_\x253c.");
    
    screen.WriteLastChar('[', ']');
    waitKey();

    screen.SetTextAttr(TEXT_BLUE | FON_RED);
    for (pos_t y = 0; y < 10; ++y)
    {
        pos_t x = 0;
        screen.GotoXY(x + 2, y + 2);
        for (; x < 20; ++x)
        {
            screen.WriteChar('0' + x + y);
        }
    }
    waitKey();

    pos_t l = 6;
    pos_t t = 4;
    pos_t r = l + 11;
    pos_t b = t + 5;

    screen.SetTextAttr(TEXT_RED | FON_BLUE);
    pos_t n = 1;

    screen.SetCursor(cursor_t::CURSOR_HIDE);
    screen.ScrollBlock(l, t, r, b, n, scroll_t::SCROLL_RIGHT);
    waitKey();
    screen.ScrollBlock(l, t, r, b, n, scroll_t::SCROLL_DOWN);
    waitKey();
    screen.ScrollBlock(l, t, r, b, n, scroll_t::SCROLL_LEFT);
    waitKey();
    screen.ScrollBlock(l, t, r, b, n, scroll_t::SCROLL_UP);
    waitKey();
    screen.SetCursor(cursor_t::CURSOR_OVERWRITE);

    ScreenBuffer cell2((size_t)r - l + 1, (size_t)b - t + 1);
    cell2.Fill(MAKE_CELL(0, TEXT_RED | FON_BLUE, 'X'));
    screen.WriteBlock(l, t, r, b, cell2);
    waitKey();

    screen.Beep();
    waitKey();

    screen.Deinit();
    LOG(INFO) << "ConsoleOutput test end";
}

int main()
{
    ConfigureLogger("m-%datetime{%Y%M%d}.log", 0x200000, false);
    //InputTest();
    OutputTest();

    LOG(INFO) << "End";
    return 0;
}