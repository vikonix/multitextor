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

#include "Editor.h"

#include "utils/logger.h"
#include "utils/Directory.h"
#include "App.h"
#include "DlgControls.h"
#include "Dialogs/StdDialogs.h"
#include "Editor.h"

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

menu_list menu0 {
    {MENU_ITEM,       "&File",     K_MENU + 1, "File menu"},
    {MENU_ITEM,       "&Edit",     K_MENU + 2, "Edit menu"},
    {MENU_ITEM,       "Bloc&k",    K_MENU + 3},
    {MENU_ITEM,       "&Search",   K_MENU + 4},
    {MENU_ITEM,       "&Tools",    K_MENU + 5},
    {MENU_ITEM,       "Windo&ws",  K_MENU + 6},
    {MENU_ITEM,       "&Help",     K_MENU + 7}
};
menu_list menu1 {
    {MENU_ITEM,       "menu1",     K_MENU + 1, "Next menu"},
    {MENU_ITEM,       "menu2",     K_F2},
    {MENU_SEPARATOR,  "menu3",     K_F3},
    {MENU_ITEM,       "menu4",     K_F4},
    {MENU_ITEM,       "menu5",     K_F5},
    {MENU_ITEM,       "menu6",     K_MENU + 1, "Next menu"},
    {MENU_ITEM,       "menu7",     K_F7}
};

sline_list sLine {
    {"",     "",     stat_color::normal},//0
    {"Key",  "",     stat_color::grayed},//1
    {"Mark", "",     stat_color::grayed},//2
    {"Rec",  "Play", stat_color::grayed},//3
    {"Ins",  "Ovr",  stat_color::normal} //4
};


/////////////////////////////////////////////////////////////////////////////
menu_list mAccess {
  {MENU_ITEM, "&F&1Exit"},
  {MENU_ITEM, "&F&2Menu"},
  {MENU_ITEM, "&F&3DlgBox"},
  {MENU_ITEM, "&F&4"},
  {MENU_ITEM, "&F&5"},
  {MENU_ITEM, "&F&6"},
  {MENU_ITEM, "&F&7"},
  {MENU_ITEM, "&F&8"},
  {MENU_ITEM, "&F&9"},
  {MENU_ITEM, "&F&1&0"}
};

class MyApp : public Application
{
public:
    virtual input_t AppProc(input_t code) override final 
    { 
        //input treatment in user function
        if (code != K_TIME)
            LOG(DEBUG) << __FUNC__ << " code=" << std::hex << code << std::dec;

        if (code == K_F2)
        {
            WndManager::getInstance().PutInput(K_MENU);
            code = 0;
        }
        else if (code == K_F3)
        {
            //code = MsgBox("Title", "Str111", "Str2222222", MBOX_OK_CANCEL_IGNORE);
            FileDialog dlg{ FileDlgMode::Open };
            auto rc = dlg.Activate();
            code = 0;
            if (rc == ID_OK)
            {
                std::filesystem::path path{dlg.s_vars.path};
                path /= dlg.s_vars.file;
                Editor ed{ path };
                ed.Load();
            }
        }

        return code; 
    } 

    virtual bool    LoadCfg()  override final
    {
        //configuration loading
        LOG(DEBUG) << __FUNC__;
        return true;
    }

    virtual bool    SaveCfg([[maybe_unused]] input_t code = 0)  override final
    { 
        //configuration saving
        LOG(DEBUG) << __FUNC__;
        return true;
    } 
};

MyApp app;
Application& Application::s_app{app};

int main()
{
    ConfigureLogger("m-%datetime{%Y%M%d}.log");
    LOG(INFO);
    LOG(INFO) << "Winman test";

    //Application& app = Application::getInstance();
    //MyApp app;
    app.Init();
    WndManager::getInstance().SetScreenSize();

    app.SetLogo(g_Logo);
    app.WriteAppName(L"TestApp");

    app.SetMenu({menu0, menu1});
    app.SetAccessMenu(mAccess);
    app.SetStatusLine(sLine);
    app.SetClock(clock_pos::bottom);
    
    app.Refresh();
    app.MainProc(K_F1);

    app.Deinit();

    LOG(INFO) << "End";
    return 0;
}