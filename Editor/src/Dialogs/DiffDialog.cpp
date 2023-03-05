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
#include "Dialogs/EditorDialogs.h"
#include "utils/Directory.h"
#include "WndManager/DlgControls.h"
#include "WndManager/App.h"
#include "EditorApp.h"
#include "utils/CpConverter.h"

using namespace _Utils;

namespace _Editor
{

/////////////////////////////////////////////////////////////////////////////
#define ID_DF_FILE1        (ID_USER + 1)
#define ID_DF_FILE2        (ID_USER + 2)
#define ID_DF_WHOLE        (ID_USER + 3)
#define ID_DF_FROMCUR      (ID_USER + 4)
#define ID_DF_INMARKED     (ID_USER + 5)
#define ID_DF_WITHOUTSPACE (ID_USER + 6)

DiffDialogVars DiffDialog::s_vars;

std::list<control> diffDialog
{
    {CTRL_TITLE,                        "File Comparing",               0,                  {},                                  1,  0, 70, 13},

    {CTRL_STATIC,                       "Compare:",                     0,                  {},                                  1,  1,  9},
    {CTRL_STATIC,                       "",                             ID_DF_FILE1,        {},                                 10,  1, 57,  7},
    {CTRL_STATIC,                       "With:",                        0,                  {},                                  1,  2,  9},
    {CTRL_STATIC,                       "",                             ID_DF_FILE2,        {},                                 10,  2, 57,  7},

    {CTRL_LINE,                         "",                             0,                  {},                                  1,  4, 66},

    {CTRL_RADIO,                        "Compare &whole files",         ID_DF_WHOLE,        &DiffDialog::s_vars.diffArea,        1,  5,  0,  0, "Compare of the whole files content"},
    {CTRL_RADIO,                        "Compare from &corrent position",ID_DF_FROMCUR,     &DiffDialog::s_vars.diffArea,        1,  6,  0,  0, "Compare from the current position to the end"},
    {CTRL_RADIO,                        "Restrict in &marked block",    ID_DF_INMARKED,     &DiffDialog::s_vars.diffArea,        1,  7,  0,  0, "Compare of the marked lines only"},
    {CTRL_CHECK,                        "Ignore diffrerences in &space",ID_DF_WITHOUTSPACE, &DiffDialog::s_vars.diffWithoutSpace,35, 5,  0,  0, "Skip spaces while compare"},
    {CTRL_CHECK,                        "Mark differ on &exit",         0,                  &DiffDialog::s_vars.markDiff,       35,  6,  0,  0, "Mark the current different lines while exit from comparition mode"},

    {CTRL_LINE,                         "",                             0,                  {},                                  1,  9, 66},
    {CTRL_DEFBUTTON | CTRL_ALIGN_RIGHT, "Compare",                      ID_OK,              {},                                 40, 10,  0,  0, ""},
    {CTRL_BUTTON | CTRL_ALIGN_RIGHT,    "Cancel",                       ID_CANCEL,          {},                                 50, 10},
};

DiffDialog::DiffDialog(bool markDiff, pos_t x, pos_t y)
    : Dialog(diffDialog, x, y)
{
    s_vars.markDiff = markDiff;
}

bool DiffDialog::OnActivate()
{
    auto wnd1 = WndManager::getInstance().GetWnd(0, 0);
    auto wnd2 = WndManager::getInstance().GetWnd(0, 1);
    if (!wnd1 || !wnd2 || wnd1->GetWndType() != wnd_t::editor || wnd2->GetWndType() != wnd_t::editor)
    {
        EditorApp::SetErrorLine("Not split view mode");
        return false;
    }

    auto ctrlPath = GetItem(ID_DF_FILE1);
    auto path = wnd1->GetFilePath();
    auto shortPath = Directory::CutPath(path, ctrlPath->GetSizeX());
    ctrlPath->SetName(shortPath);

    ctrlPath = GetItem(ID_DF_FILE2);
    path = wnd2->GetFilePath();
    shortPath = Directory::CutPath(path, ctrlPath->GetSizeX());
    ctrlPath->SetName(shortPath);

    auto edWnd1 = dynamic_cast<EditorWnd*>(wnd1);
    auto edWnd2 = dynamic_cast<EditorWnd*>(wnd2);
    if (!edWnd1->IsMarked() || !edWnd2->IsMarked())
    {
        if (s_vars.diffArea == 2)
        {
            s_vars.diffArea = 0;
            auto item = GetItem(ID_DF_WHOLE);
            auto ctrl = std::dynamic_pointer_cast<CtrlRadio>(item);
            if (ctrl)
                ctrl->SetCheck();
        }
        GetItem(ID_DF_INMARKED)->SetMode(CTRL_DISABLED);
    }

    return true;
}

bool DiffDialog::OnClose(int /*id*/)
{
    return true;
}

} //namespace _Editor
