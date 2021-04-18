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

using namespace _Utils;

namespace _Editor
{

/////////////////////////////////////////////////////////////////////////////
#define ID_DP_PATH         (ID_USER +  1)
#define ID_DP_NAME         (ID_USER +  2)
#define ID_DP_INFO         (ID_USER +  3)
#define ID_DP_TYPE         (ID_USER +  4)
#define ID_DP_CP           (ID_USER +  5)
#define ID_DP_EOL          (ID_USER +  6)
#define ID_DP_TAB          (ID_USER +  7)
#define ID_DP_TAB_CONVERT  (ID_USER +  8)
#define ID_DP_TAB_SAVE     (ID_USER +  9)
#define ID_DP_TAB_SHOW     (ID_USER + 10)
#define ID_DP_LOG          (ID_USER + 11)
#define ID_DP_RO           (ID_USER + 12)

PropertiesVars PropertiesDialog::s_vars;

std::list<control> propertiesDialog
{
    {CTRL_TITLE,                        "File Properties",      0,                  {},                                 1,  0, 70, 17},

    {CTRL_STATIC,                       "",                     ID_DP_PATH,         {},                                 1,  1, 66,  7},
    {CTRL_STATIC,                       "",                     ID_DP_NAME,         {},                                 1,  2, 66,  7},
    {CTRL_STATIC,                       "",                     ID_DP_INFO,         {},                                 1,  3, 66,  7},
    {CTRL_LINE,                         "",                     0,                  {},                                 1,  5, 66},
    {CTRL_STATIC,                       "&File type:",          0,                  {},                                 1,  6, 14},
    {CTRL_DROPLIST,                     "",                     ID_DP_TYPE,         &PropertiesDialog::s_vars.type,    15,  6, 17,  7, "Select file type"},
    {CTRL_STATIC,                       "Code &page:",          0,                  {},                                 1,  7, 14},
    {CTRL_DROPLIST,                     "",                     ID_DP_CP,           &PropertiesDialog::s_vars.cp,      15,  7, 17,  7, "Select encoding page"},
    {CTRL_CHECK,                        "&Read only",           ID_DP_RO,           &PropertiesDialog::s_vars.ro,       1,  9, 30,  1, "Protect file from changing"},
    {CTRL_CHECK,                        "&Log file",            ID_DP_LOG,          &PropertiesDialog::s_vars.log,      1, 10, 30,  1, "File will be reload without confirmation if it changed"},
    {CTRL_STATIC,                       "&End of line:",        0,                  {},                                 35,  6, 13},
    {CTRL_DROPLIST,                     "",                     ID_DP_EOL,          &PropertiesDialog::s_vars.eol,      49,  6, 18,  6, "Select the end of line type"},
    {CTRL_STATIC,                       "&Tab size:",           0,                  {},                                 35,  8, 13},
    {CTRL_EDIT,                         "",                     ID_DP_TAB,          {},                                 64,  8,  3,  7, "Input tabulation size (1-10)"},
    {CTRL_RADIO,                        "Convert tabs to &space",ID_DP_TAB_CONVERT, &PropertiesDialog::s_vars.saveTab,  35,  9, 30,  1, "Convert all tabulations to space"},
    {CTRL_RADIO,                        "&Use tabs as space",   ID_DP_TAB_SAVE,     &PropertiesDialog::s_vars.saveTab,  35, 10, 30,  1, "Save tabulations"},
    {CTRL_CHECK,                        "S&how tabs",           ID_DP_TAB_SHOW,     &PropertiesDialog::s_vars.showTab,  35, 11, 30,  1, "Highlight tabulations"},

    {CTRL_LINE,                         "",                     0,                  {},                                  1, 13, 66},
    {CTRL_DEFBUTTON | CTRL_ALIGN_RIGHT, "Ok",                   ID_OK,              {},                                 50, 14,  0,  0, "Apply changes and reload file if need"},
    {CTRL_BUTTON | CTRL_ALIGN_RIGHT,    "Cancel",               ID_CANCEL,          {},                                 60, 14}
};

PropertiesDialog::PropertiesDialog(pos_t x, pos_t y)
    : Dialog(propertiesDialog, x, y)
{
}

bool PropertiesDialog::OnActivate()
{
    auto ctrlPath = GetItem(ID_DP_PATH);
    size_t ctrlSize = ctrlPath->GetSizeX();

    Wnd* wnd = WndManager::getInstance().GetWnd();
    auto path = wnd->GetFilePath();
    if (path.u16string().size() < ctrlSize)
        ctrlPath->SetName(path.u8string());
    else
    {
        ctrlPath->SetName(path.parent_path().u8string());
        GetItem(ID_DP_NAME)->SetName(path.filename().u8string());
    }

    auto ftime = std::filesystem::last_write_time(path);
    std::time_t cftime = to_time_t(ftime);
    std::tm tm = *std::localtime(&cftime);

    std::stringstream sinfo;
    sinfo << std::put_time(&tm, "%d %b %Y %H:%M:%S");

    auto size = std::filesystem::file_size(path);
    if (size > 10 * 1024 * 1024)
    {
        auto s = size / (1024 * 1024);
        sinfo << " " << std::setw(10) << s << "M";
    }
    else if (size > 100 * 1024)
    {
        auto s = size / 1024;
        sinfo << " " << std::setw(10) << s << "K";
    }
    else
    {
        sinfo << " " << std::setw(11) << size;
    }

    GetItem(ID_DP_INFO)->SetName(sinfo.str());

    auto ctrl = GetItem(ID_DP_TYPE);
    auto listPtr = std::dynamic_pointer_cast<CtrlDropList>(ctrl);
    for (const auto& str : s_vars.typeList)
    {
        listPtr->AppendStr(str);
        if (str == s_vars.typeName)
            listPtr->SetSelect(listPtr->GetStrCount() - 1);
    }

    ctrl = GetItem(ID_DP_CP);
    listPtr = std::dynamic_pointer_cast<CtrlDropList>(ctrl);
    for (const auto& str : s_vars.cpList)
    {
        listPtr->AppendStr(str);
        if (str == s_vars.cpName)
            listPtr->SetSelect(listPtr->GetStrCount() - 1);
    }

    ctrl = GetItem(ID_DP_EOL);
    listPtr = std::dynamic_pointer_cast<CtrlDropList>(ctrl);
    for (const auto& str : s_vars.eolList)
        listPtr->AppendStr(str);
    listPtr->SetSelect(s_vars.eol);

    GetItem(ID_DP_TAB)->SetName(std::to_string(std::min(s_vars.tabSize, static_cast<size_t>(10))));

    return true;
}

bool PropertiesDialog::OnClose(int id)
{
    if (id == ID_OK)
    {
        auto tabStr = GetItem(ID_DP_TAB)->GetName();

        size_t tabSize{};
        bool badFormat{};
        try
        {
            tabSize = std::stoul(tabStr);
            if (std::to_string(tabSize) != tabStr)
                badFormat = true;
        }
        catch (...)
        {
            badFormat = true;
        }
        if (badFormat || tabSize > 10)
        {
            Application::getInstance().SetErrorLine("Bad tab size");
            SelectItem(ID_DP_TAB);
            Refresh();
            return false;
        }

        s_vars.tabSize = tabSize;
        s_vars.typeName = GetItem(ID_DP_TYPE)->GetName();
        s_vars.cpName = GetItem(ID_DP_CP)->GetName();

        auto ctrl = GetItem(ID_DP_EOL);
        auto listPtr = std::dynamic_pointer_cast<CtrlDropList>(ctrl);
        if (listPtr)
            s_vars.eol = listPtr->GetSelected();
    }
    return true;
}

} //namespace _Editor
