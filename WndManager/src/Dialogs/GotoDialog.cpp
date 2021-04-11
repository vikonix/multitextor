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
#include "Dialogs/StdDialogs.h"
#include "DlgControls.h"
#include "App.h"


/////////////////////////////////////////////////////////////////////////////
#define ID_GL_NUMBER (ID_USER + 1)

std::string GotoDialog::m_line{};

std::list<control> gotoDialog
{
    {CTRL_TITLE,                        "Go to Line",       0, nullptr,              1, 0, 34,  7},

    {CTRL_STATIC,                       "Line number:",     0, nullptr,              1, 1, 15},
    {CTRL_EDIT,                         "",      ID_GL_NUMBER, &GotoDialog::m_line, 16, 1, 15,  0, "Input line number"},
    {CTRL_LINE,                         "",                 0, nullptr,              1, 3, 30},

    {CTRL_DEFBUTTON | CTRL_ALIGN_RIGHT, "Goto",         ID_OK, nullptr,             10, 4,  0,  0, "Goto selected line"},
    {CTRL_BUTTON | CTRL_ALIGN_RIGHT,    "Cancel",   ID_CANCEL, nullptr,             21, 4}
};

GotoDialog::GotoDialog(size_t maxLine, pos_t x, pos_t y)
    : Dialog(gotoDialog, x, y)
    , m_maxLine{ maxLine }
{
}

bool GotoDialog::OnClose(int id)
{
    if (id == ID_OK)
    {
        auto str = GetItem(ID_GL_NUMBER)->GetName();

        std::string error;
        try 
        {
            auto line = std::stoull(str);
            if (line > 0 && line <= m_maxLine)
                return true;
            else
                error = "Line number out of range";
        }
        catch (...)
        {
            error = "Invalid parameter";
        }
        
        Application::getInstance().SetErrorLine(error);
        SelectItem(ID_GL_NUMBER);
        Refresh();
        return false;
    }

    return true;
}
