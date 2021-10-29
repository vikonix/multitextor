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
#include "Dialogs/EditorDialogs.h"
#include "WndManager/DlgControls.h"
#include "WndManager/App.h"
#include "WndManager/WndManager.h"
#include "EditorWnd.h"

namespace _Editor
{

/////////////////////////////////////////////////////////////////////////////
#define ID_FF_SEARCH   (ID_USER +  1)
#define ID_FF_REPLACE  (ID_USER +  2)
#define ID_FF_SREPLACE (ID_USER +  3)
#define ID_FF_SMASK    (ID_USER +  4)
#define ID_FF_MASK     (ID_USER +  5)
#define ID_FF_PATH     (ID_USER +  6)
#define ID_FF_FILELIST (ID_USER +  7)
#define ID_FF_DIRLIST  (ID_USER +  8)
#define ID_FF_CASE     (ID_USER +  9)
#define ID_FF_REVERSE  (ID_USER + 10)
#define ID_FF_SUBDIR   (ID_USER + 11)
#define ID_FF_OPEN     (ID_USER + 12)
#define ID_FF_PROMPT   (ID_USER + 13)
#define ID_FF_INMARKED (ID_USER + 14)
#define ID_FF_CP       (ID_USER + 15)

FindReplaceVars FindDialog::s_vars;

std::list<control> findDialog 
{
    {CTRL_TITLE,                        "",                         0,              {},                              1, 0, 70, 12},

    {CTRL_STATIC,                       "&Search for:",             0,              {},                              1, 1, 14},
    {CTRL_EDITDROPLIST,                 "",                         ID_FF_SEARCH,   &FindDialog::s_vars.findStr,    15, 1, 52,  7, "Input string for search"},
    {CTRL_STATIC,                       "&Replace with:",           ID_FF_SREPLACE, {},                              1, 2, 14},
    {CTRL_EDITDROPLIST,                 "",                         ID_FF_REPLACE,  &FindDialog::s_vars.replaceStr, 15, 2, 52,  7, "Input string for replace"},
    {CTRL_CHECK,                        "C&ase sensitive",          2,              &FindDialog::s_vars.checkCase,   1, 4,  0,  0, "Search case sensitive or not"},
    {CTRL_CHECK,                        "&Whole word",              3,              &FindDialog::s_vars.findWord,    1, 5,  0,  0, "Search whole word or phrase"},
    {CTRL_CHECK,                        "Restrict in &marked lines",ID_FF_INMARKED, &FindDialog::s_vars.inSelected,  1, 6,  0,  0, "Find/Replace in marked lines only"},
    {CTRL_CHECK,                        "Reverse &direction",       ID_FF_REVERSE,  &FindDialog::s_vars.directionUp,35, 4,  0,  0, "Search in up direction"},
    {CTRL_CHECK,                        "Replace without &prompt",  ID_FF_PROMPT,   &FindDialog::s_vars.noPrompt,   35, 4,  0,  0, "Replace in whole file without prompt"},

    {CTRL_LINE,                         "",                         0,              {},                              1, 8, 66},
    {CTRL_DEFBUTTON | CTRL_ALIGN_RIGHT, "",                         ID_OK,          {},                             43, 9,  0,  0, "Start search process"},
    {CTRL_BUTTON | CTRL_ALIGN_RIGHT,    "Cancel",                   ID_CANCEL,      {},                             57, 9}
};

FindDialog::FindDialog(bool replace, pos_t x, pos_t y)
    : Dialog(findDialog, x, y)
    , m_replace{ replace }
{
    s_vars.replaceMode = replace;
}

bool FindDialog::OnActivate()
{
    Wnd* wnd = WndManager::getInstance().GetWnd();
    if (wnd && wnd->GetWndType() != wnd_t::editor)
        return false;

    auto editor = dynamic_cast<EditorWnd*>(wnd);
    
    std::u16string curWord;
    bool cur = editor->GetWord(curWord);
    auto word = utf8::utf16to8(curWord);

    auto seach = GetItem(ID_FF_SEARCH);
    auto ctrlSearch = std::dynamic_pointer_cast<CtrlEditDropList>(seach);
    if (ctrlSearch)
    {
        if(cur)
            ctrlSearch->AppendStr(word);
        for(auto& str : s_vars.findList)
            if(str != word)
                ctrlSearch->AppendStr(str);
    }

    if (m_replace)
    {
        seach = GetItem(ID_FF_REPLACE);
        ctrlSearch = std::dynamic_pointer_cast<CtrlEditDropList>(seach);
        if (ctrlSearch)
        {
            if (cur)
                ctrlSearch->AppendStr(word);
            for (auto& str : s_vars.replaceList)
                if (str != word)
                    ctrlSearch->AppendStr(str);
        }
    }

    size_t begin, end;
    if (!editor->IsMarked() || !editor->GetSelectedLines(begin, end) || begin == end)
    {
        auto inmarked = GetItem(ID_FF_INMARKED);
        auto ctrlInmarked = std::dynamic_pointer_cast<CtrlCheck>(inmarked);
        if (ctrlInmarked)
        {
            ctrlInmarked->SetMode(CTRL_DISABLED);
            ctrlInmarked->SetCheck(0);
        }
    }

    if (!m_replace)
    {
        GetItem(0)->SetName("Find");
        GetItem(ID_OK)->SetName("Find");

        GetItem(ID_FF_SREPLACE)->SetMode(CTRL_HIDE);
        GetItem(ID_FF_REPLACE)->SetMode(CTRL_HIDE);
        GetItem(ID_FF_PROMPT)->SetMode(CTRL_HIDE);
    }
    else
    {
        GetItem(0)->SetName("Find And Repalece");
        GetItem(ID_OK)->SetName("Replace");

        GetItem(ID_FF_REVERSE)->SetMode(CTRL_HIDE);
    }

    return true;
}


bool FindDialog::OnClose(int id)
{
    if (id == ID_OK)
    {
        auto seach = GetItem(ID_FF_SEARCH);
        auto ctrlSearch = std::dynamic_pointer_cast<CtrlEditDropList>(seach);
        if (ctrlSearch)
        {
            auto str = ctrlSearch->GetName();
            if (str.empty())
            {
                Application::getInstance().SetErrorLine("Search string empty");
                SelectItem(ID_FF_SEARCH);
                Refresh();
                return false;
            }
            if (str.size() > 2)
                ctrlSearch->AddStr(0, str);
            s_vars.findStrW = utf8::utf8to16(str);

            size_t n = ctrlSearch->GetStrCount();
            s_vars.findList.clear();
            for (size_t i = 0; i < n; ++i)
            {
                _TRY(s_vars.findList.emplace(ctrlSearch->GetStr(i)))
                if (s_vars.findList.size() == MAX_FR_LIST)
                    break;
            }
        }
        
        if (m_replace)
        {
            seach = GetItem(ID_FF_REPLACE);
            auto ctrlReplace = std::dynamic_pointer_cast<CtrlEditDropList>(seach);
            if (ctrlReplace)
            {
                auto str = ctrlReplace->GetName();
                if (str.empty())
                {
                    Application::getInstance().SetErrorLine("Replace string empty");
                    SelectItem(ID_FF_REPLACE);
                    Refresh();
                    return false;
                }
                if (str.size() > 2)
                    ctrlReplace->AddStr(0, str);
                s_vars.replaceStrW = utf8::utf8to16(str);

                size_t n = ctrlReplace->GetStrCount();
                s_vars.replaceList.clear();
                for (size_t i = 0; i < n; ++i)
                {
                    _TRY(s_vars.replaceList.emplace(ctrlReplace->GetStr(i)))
                    if (s_vars.replaceList.size() == MAX_FR_LIST)
                        break;
                }
            }
        }
    }

    return true;
}

} //namespace _Editor
