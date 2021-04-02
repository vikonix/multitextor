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
#include "utfcpp/utf8.h"
#include "utils/Directory.h"
#include "Dialogs/EditorDialogs.h"
#include "DlgControls.h"
#include "WndManager.h"
#include "EditorApp.h"


#define ID_WL_WNDLIST  (ID_USER +  1)
#define ID_WL_COUNT    (ID_USER +  2)
#define ID_WL_CLOSE    (ID_USER +  3)

std::list<control> dlgWindowList
{
    {CTRL_TITLE,                        "Windows List", 0,              nullptr,             1,  0, 70, 21},

    {CTRL_LIST,                         "",             ID_WL_WNDLIST,  (size_t*)nullptr,    0,  0, 68, 18, "Select window"},
    {CTRL_STATIC,                       "",             ID_WL_COUNT,    nullptr,             1, 18, 35},

    {CTRL_DEFBUTTON | CTRL_ALIGN_LEFT,  "Select",       ID_OK,          nullptr,            36, 18,  0,  0, "Activate selected window"},
    {CTRL_BUTTON | CTRL_ALIGN_LEFT,     "Close",        ID_WL_CLOSE,    nullptr,            47, 18,  0,  0, "Close selected window"},
    {CTRL_BUTTON | CTRL_ALIGN_LEFT,     "Cancel",       ID_CANCEL,      nullptr,            57, 18}
};

WindowListDialog::WindowListDialog(WindowsDlgMode mode, pos_t x, pos_t y)
    : Dialog(dlgWindowList, x, y)
    , m_mode{ mode }
{
}

bool WindowListDialog::OnActivate()
{
    Wnd* pWnd = WndManager::getInstance().GetWnd();
    if (!pWnd)
    {
        BeginPaint();
        EditorApp::SetErrorLine("All windows are closed");
        StopPaint();
        return false;
    }

    bool skip{};
    if (m_mode == WindowsDlgMode::CompareWith)
        skip = true;

    size_t count;
    if ((count = GetWndList(skip)) == 0)
    {
        BeginPaint();
        EditorApp::SetErrorLine("No windows with marked block");
        StopPaint();
        return false;
    }

    if (m_mode == WindowsDlgMode::CopyFrom)
    {
        GetItem(0)->SetName("Copy Selected Block");
        GetItem(ID_OK)->SetName("Copy");
        GetItem(ID_OK)->SetHelpLine("Copy selected block from another window");
        GetItem(ID_WL_CLOSE)->SetName("Unmark");
        GetItem(ID_WL_CLOSE)->SetHelpLine("Clear selection in window");
    }
    else if (m_mode == WindowsDlgMode::MoveFrom)
    {
        GetItem(0)->SetName("Move Selected Block");
        GetItem(ID_OK)->SetName("Move");
        GetItem(ID_OK)->SetHelpLine("Move selected block from another window");
        GetItem(ID_WL_CLOSE)->SetName("Unmark");
        GetItem(ID_WL_CLOSE)->SetHelpLine("Clear selection in window");
    }
    else if (m_mode == WindowsDlgMode::CompareWith)
    {
        GetItem(0)->SetName("Select window for comparing");
        GetItem(ID_OK)->SetHelpLine("Show selected windows in splited view");
        GetItem(ID_WL_CLOSE)->SetMode(CTRL_HIDE);
    }

    std::stringstream sstr;
    sstr << count << " window(s)";
    GetItem(ID_WL_COUNT)->SetName(sstr.str());

    return true;
}

size_t WindowListDialog::GetWndList(bool skip)
{
    m_wndList.clear();

    auto listCtrl = GetItem(ID_WL_WNDLIST);
    auto ListPtr = std::dynamic_pointer_cast<CtrlList>(listCtrl);
    ListPtr->Clear();

    size_t count{};
    Wnd* wnd;
    while ((wnd = WndManager::getInstance().GetWnd(count, 0)) != nullptr)
    {
        ++count;
        if (skip)
        {
            skip = false;
            continue;
        }
        if (wnd->GetWndType() != wnd_t::editor)
            continue;
        if (m_mode == WindowsDlgMode::List || m_mode == WindowsDlgMode::CompareWith || dynamic_cast<EditorWnd*>(wnd)->IsMarked())
        {
            auto path = wnd->GetFilePath();// .relative_path();
            auto shortPath = Directory::CutPath(path, listCtrl->GetSizeX());
            m_wndList[shortPath] = wnd;
        }
    }

    for(auto& [path, w] : m_wndList)
        ListPtr->AppendStr(path);

    return m_wndList.size();
}

input_t WindowListDialog::DialogProc(input_t code)
{
    return code;
}

bool WindowListDialog::OnClose(int id)
{
    return true;
}


