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
#include "EditorApp.h"
#include "Dialogs/StdDialogs.h"


sline_list sLine{
    {"",     "",     stat_color::normal},//0
    {"Key",  "",     stat_color::grayed},//1
    {"Mark", "",     stat_color::grayed},//2
    {"Rec",  "Play", stat_color::grayed},//3
    {"Ins",  "Ovr",  stat_color::normal} //4
};

bool EditorApp::Init()
{
    Application::Init();
    SetStatusLine(sLine);
    return true;
}

void EditorApp::Deinit()
{
    Application::Deinit();
}

bool EditorApp::StatusWaitKey(bool wait)
{
    if (s_wait != wait)
    {
        ChangeStatusLine(1, wait ? stat_color::normal : stat_color::grayed);
        if (wait)
            ChangeStatusLine(0, "Press next key of sequence");
        else
            ChangeStatusLine(0);
        s_wait = wait;
    }

    return true;
}

bool EditorApp::StatusMark(mark_status mark)
{
    static mark_status s_mark{};
    if (s_mark != mark)
    {
        //this will clear all other help line
        getInstance().ChangeStatusLine(2, mark != mark_status::no ? stat_color::normal : stat_color::grayed);
        if (mark == mark_status::mark_by_key)
            getInstance().ChangeStatusLine(0, "Press F4 for to end selection");
        else
            getInstance().ChangeStatusLine(0);
        s_mark = mark;
    }

    return 0;
}

input_t EditorApp::AppProc(input_t code)
{ 
    //input treatment in user function
    LOG_IF(code != K_TIME, DEBUG) << __FUNC__ << " code=" << std::hex << code << std::dec;

    if (code == K_EXIT)
    {
        m_editors.clear();
    }
//    else if (code == K_MENU)
//    {
//        WndManager::getInstance().PutInput(K_MENU);
//        code = 0;
//    }
    else if (code == K_APP_DLG_OPEN)
    {
        FileDialog dlg{ FileDlgMode::Open };
        auto rc = dlg.Activate();
        code = 0;
        if (rc == ID_OK)
        {
            std::filesystem::path path{dlg.s_vars.path};
            path /= dlg.s_vars.file;

            auto editor = std::make_shared<EditorWnd>();
            editor->Show(true, -1);
            std::string type = dlg.s_vars.type < dlg.s_vars.typeList.size() ? *std::next(dlg.s_vars.typeList.cbegin(), dlg.s_vars.type) : "";
            editor->SetFileName(path, false, type);
                
            m_editors[path.u8string()] = editor;
        }
    }

    return code; 
} 

bool EditorApp::LoadCfg()
{
    //configuration loading
    LOG(DEBUG) << __FUNC__;
    return true;
}

bool EditorApp::SaveCfg([[maybe_unused]] input_t code)
{ 
    //configuration saving
    LOG(DEBUG) << __FUNC__;
    return true;
} 
