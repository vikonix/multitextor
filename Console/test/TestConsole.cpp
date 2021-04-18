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
#ifdef USE_VLD
  #include <vld.h>
#endif

#include "utils/logger.h"
#include "Console/Console.h"

#include <iostream>

using namespace _Utils;
using namespace _Console;

void ConsoleTest()
{
    std::cout << "Console test" << std::endl;
    LOG(INFO);
    LOG(INFO) << "Console test";

    Console console;

    console.Init();
    console.SetScreenSize(MAX_COORD, MAX_COORD);

    auto waitKey = [&console]() {
        console.Flush();
        while (1)
        {
            console.InputPending();
            input_t key = console.GetInput();
            LOG_IF(key, INFO) << "  " << ConsoleInput::CastKeyCode(key);
            if (key == K_SPACE)
                break;
        }
    };

    console.WriteConsoleTitle("Console Screen Ш");
    console.ClrScr();
    console.SetCursor(cursor_t::CURSOR_NORMAL);
    console.GotoXY(0, 0);
    waitKey();

    console.SetTextAttr(TEXT_BLUE);
    console.WriteChar(u'\x428'); // russian sh:'Ш'

    console.SetTextAttr(TEXT_RED);
    console.WriteChar('[');
    for(char16_t i = 1; i < ACS_MAX; ++i)
        console.WriteChar(i);
    console.WriteChar(']');

    console.SetTextAttr(COLOR_INVERSE(TEXT_GREEN));
    console.WriteStr(u"Text\x428_\x253c.");
    
    console.WriteLastChar('[', ']');
    waitKey();

    console.SetTextAttr(TEXT_BLUE | FON_RED);
    for (pos_t y = 0; y < 10; ++y)
    {
        pos_t x = 0;
        console.GotoXY(x + 2, y + 2);
        for (; x < 20; ++x)
        {
            console.WriteChar('0' + x + y);
        }
    }
    waitKey();

    pos_t l = 6;
    pos_t t = 4;
    pos_t r = l + 11;
    pos_t b = t + 5;

    console.SetTextAttr(TEXT_RED | FON_BLUE);
    pos_t n = 1;

    console.SetCursor(cursor_t::CURSOR_HIDE);
    console.ScrollBlock(l, t, r, b, n, scroll_t::SCROLL_RIGHT);
    waitKey();
    console.ScrollBlock(l, t, r, b, n, scroll_t::SCROLL_DOWN);
    waitKey();
    console.ScrollBlock(l, t, r, b, n, scroll_t::SCROLL_LEFT);
    waitKey();
    console.ScrollBlock(l, t, r, b, n, scroll_t::SCROLL_UP);
    waitKey();
    console.SetCursor(cursor_t::CURSOR_OVERWRITE);

    ScreenBuffer cell2(static_cast<size_t>(r) - l + 1, static_cast<size_t>(b) - t + 1);
    cell2.Fill(MAKE_CELL(0, TEXT_RED | FON_BLUE, 'X'));
    console.WriteBlock(l, t, r, b, cell2);
    waitKey();

    console.Beep();
    waitKey();

    console.Deinit();
    LOG(INFO) << "ConsoleOutput test end";
}

int main()
{
    ConfigureLogger("m-%datetime{%Y%M%d}.log", 0x200000, false);
    ConsoleTest();

    LOG(INFO) << "End";
    return 0;
}