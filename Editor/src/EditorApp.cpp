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


sline_list g_statusLine
{
    {"",     "",     stat_color::normal},//0
    {"Key",  "",     stat_color::grayed},//1
    {"Mark", "",     stat_color::grayed},//2
    {"Rec",  "Play", stat_color::grayed},//3
    {"Ins",  "Ovr",  stat_color::normal} //4
};

bool EditorApp::Init()
{
    Application::Init();
    SetStatusLine(g_statusLine);
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

std::string EditorApp::GetKeyName(input_t code) const
{
    std::vector<input_t> codeKeys;
    for (auto it = g_defaultAppKeyMap.cbegin(); it != g_defaultAppKeyMap.cend(); ++it)
    {
        const auto& keys = *it;
        ++it;
        const auto& cmd = *it;


        if (cmd.size() == 1 && code == cmd[0])
        {
            codeKeys = keys;
            break;
        }
    }
    if(codeKeys.empty())
        for (auto it = g_defaultEditKeyMap.cbegin(); it != g_defaultEditKeyMap.cend(); ++it)
        {
            const auto& key = *it;
            ++it;
            const auto& cmd = *it;


            if (cmd.size() == 1 && code == cmd[0])
            {
                codeKeys = key;
                break;
            }
        }
    if (codeKeys.empty())
        return {};

    std::string keyNames;
    for (auto key : codeKeys)
    {
        if (!keyNames.empty())
            keyNames += " ";
        auto keyType = key & K_TYPEMASK;
        auto keyCmd = key & 0xffff0000;
        auto keyCode = K_GET_CODE(key);

        if (key & K_SHIFT)
            keyNames += "Shift+";
        if (key & K_CTRL)
            keyNames += "Ctrl+";
        if (key & K_ALT)
            keyNames += "Alt+";

        bool found{};
        if (0 != keyType)
        {
            auto it = g_CmdNames.find(keyType);
            if (it != g_CmdNames.end())
            {
                keyNames += it->second;
                found = true;
            }
        }
        if(!found && 0 != keyType && 0 != keyCmd)
        {
            auto it = g_CmdNames.find(keyCmd);
            if (it != g_CmdNames.end())
            {
                keyNames += it->second;
                found = true;
            }
        }
        if (!found && 0 != keyCode)
        {
            auto it = g_CmdNames.find(keyCode);
            if (it != g_CmdNames.end())
            {
                keyNames += it->second;
                found = true;
            }
        }
        if(!found && keyCode > ' ')
            keyNames += static_cast<char>(keyCode);
    }

    return keyNames;
}

bool EditorApp::CloseWindow(Wnd* wnd)
{
    if(auto it = m_editors.find(wnd); it != m_editors.end())
    {
        //LOG(DEBUG) << "CloseWindows " << wnd;
        m_editors.erase(it);
        return true;
    }

    return false;
}

input_t EditorApp::AppProc(input_t code)
{ 
    //input treatment in user function
    //LOG_IF(code != K_TIME, DEBUG) << __FUNC__ << " code=" << std::hex << code << std::dec;

    if (code == K_INSERT)
    {
        if (m_insert)
        {
            m_insert = false;
            SwapStatusLine(4);
        }
        else
        {
            m_insert = true;
            SwapStatusLine(4);
        }

        return 0;
    }
    else if (code == K_EXIT)
    {
        m_editors.clear();
    }
    else if(code >= K_APP && code <= K_APP_END)
    {
        AppCmd cmd = static_cast<AppCmd>(code);
        if (code >= K_APP_BOOKMARK && code < K_APP_FILE_RECENT)
            cmd = K_APP_BOOKMARK;
        else if (code >= K_APP_FILE_RECENT && code < K_APP_SESSION_RECENT)
            cmd = K_APP_FILE_RECENT;
        else if (code > K_APP_SESSION_RECENT)
            cmd = K_APP_SESSION_RECENT;

        if (auto it = s_funcMap.find(cmd); it != s_funcMap.end())
        {
            auto& [k, func] = *it;
            [[maybe_unused]]bool rc = func(this, code);
            return 0;
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
