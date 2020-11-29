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
#ifdef _DEBUG
  #ifdef USE_VLD
    #include <vld.h>
  #endif
#endif

#include "utils/logger.h"
#include "Console.h"
#include "WndManager.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
Logo g_Logo {
  ColorScreen,
  ColorScreen,
  '-',
  -1, -1, 
  {
      "----------  ----",
      "--------- # ----",
      "-------- ## ----",
      "------- ###   --",
      "------ #### # --",
      "----- #### ## --",
      "---- #### ###   ",
      "--- #### #### # ",
      "-- #### #### ## ",
      "- #### #### ### ",
      " #### #### #### "
  }
};


int main()
{
    ConfigureLogger("m-%datetime{%Y%M%d}.log", 0x200000, false);
    LOG(INFO);
    LOG(INFO) << "Winman test";

    WndManager& manager = WndManager::getInstance();

    manager.Init();
    manager.SetLogo(&g_Logo);
    manager.WriteConsoleTitle();
    manager.Refresh();

    manager.ShowInputCursor(cursor_t::CURSOR_NORMAL, 10, 10);
    //manager.Beep();
    manager.CheckInput(5s);

    FrameWnd wnd{20, 5, 20, 10, BORDER_FULL };
    wnd.Show();
    manager.Refresh();
    manager.CheckInput(5s);

    wnd.Hide();
    manager.Refresh();
    manager.CheckInput(5s);

    manager.Deinit();

    LOG(INFO) << "End";
    return 0;
}