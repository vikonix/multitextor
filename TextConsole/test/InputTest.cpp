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

#include <iostream>
#include <chrono>


int main()
{
    ConfigureLogger("m-%datetime{%Y%M%d}.log", 0x200000, true);

    std::cout << "ConsoleInput test" << std::endl;
    LOG(INFO);
    LOG(INFO) << "ConsoleInput test";

    {
#ifdef WIN32
        InputWin32 console;
#else
        InputTTY console;
#endif
        console.Init();

        while(1)
        {
            using namespace std::chrono_literals;
            console.InputPending(500ms);
            input_t key = console.GetInput();

            LOG_IF(key, INFO) << "  " << ConsoleInput::CastKeyCode(key);
            if (key == K_EXIT || key == K_ALT + 'X')
                break;
        } 
        
        console.Deinit();
    }

    LOG(INFO) << "ConsoleInput test end";
    return 0;
}