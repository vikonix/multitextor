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
#include "utfcpp/utf8.h"
#include "WndManager/DlgControls.h"
#include "WndManager/App.h"
#include "WndManager/WndManager.h"
#include "utils/CpConverter.h"
#include "EditorWnd.h"
#include "EditorApp.h"

using namespace _Utils;

namespace _Editor 
{

/////////////////////////////////////////////////////////////////////////////
FindFileDialogVars FindFileDialog::s_vars;

#define ID_FF_SEARCH   (ID_USER +  1)
#define ID_FF_REPLACE  (ID_USER +  2)
#define ID_FF_SREPLACE (ID_USER +  3)
#define ID_FF_SMASK    (ID_USER +  4)
#define ID_FF_MASK     (ID_USER +  5)
#define ID_FF_PATH     (ID_USER +  6)
#define ID_FF_FILELIST (ID_USER +  7)
#define ID_FF_DIRLIST  (ID_USER +  8)
#define ID_FF_CASE     (ID_USER +  9)
#define ID_FF_SUBDIR   (ID_USER + 10)
#define ID_FF_OPEN     (ID_USER + 11)
#define ID_FF_PROMPT   (ID_USER + 12)
#define ID_FF_INMARKED (ID_USER + 13)
#define ID_FF_CP       (ID_USER + 14)

std::list<control> findFileDialog 
{
    {CTRL_TITLE,                        "",                             0,              {},                                  1,  0, 70, 21},

    {CTRL_STATIC,                       "&Search for:",                 0,              {},                                  1,  0, 14},
    {CTRL_EDITDROPLIST,                 "",                             ID_FF_SEARCH,   &FindDialog::s_vars.findStr,        15,  0, 52,  7, "Input string for search"},
    {CTRL_STATIC,                       "&Replace with:",               ID_FF_SREPLACE, {},                                  1,  1, 14},
    {CTRL_EDITDROPLIST,                 "",                             ID_FF_REPLACE,  &FindDialog::s_vars.replaceStr,     15,  1, 52,  7, "Input string for replace"},
    {CTRL_STATIC,                       "File &mask:",                  ID_FF_SMASK,    {},                                  1,  3, 14},
    {CTRL_EDITDROPLIST,                 "",                             ID_FF_MASK,     &FileDialog::s_vars.file,           15,  3, 52,  7, "Input file mask for file search"},

    {CTRL_STATIC | CTRL_NOCOLOR,        "",                             ID_FF_PATH,     {},                                  1,  4, 66},
    {CTRL_LIST,                         "&Directories",                 ID_FF_DIRLIST,  {},                                  0,  5, 19, 14, "Select directory"},
    {CTRL_LIST,                         "&Files",                       ID_FF_FILELIST, {},                                 19,  5, 34,  8, "Select file name"},

    {CTRL_DEFBUTTON | CTRL_ALIGN_RIGHT, "",                             ID_OK,          {},                                 54,  6,  0,  0, "Start search process"},
    {CTRL_BUTTON | CTRL_ALIGN_RIGHT,    "Cancel",                       ID_CANCEL,      {},                                 54,  8},
    {CTRL_LINE,                         "",                             0,              {},                                 54, 12, 13},

    {CTRL_CHECK,                        "C&ase sensitive",              ID_FF_CASE,     &FindDialog::s_vars.checkCase,      20, 13,  0,  0, "Search case sensitive or not"},
    {CTRL_CHECK,                        "&Whole word",                  3,              &FindDialog::s_vars.findWord,       20, 14,  0,  0, "Search whole word or phrase"},
    {CTRL_CHECK,                        "Search in s&ub-directories",   ID_FF_SUBDIR,   &FindFileDialog::s_vars.recursive,  20, 15,  0,  0, "Recursive search in subdirectories or not"},
    {CTRL_CHECK,                        "Search in &opened files only", ID_FF_OPEN,     &FindFileDialog::s_vars.inOpen,     20, 16,  0,  0, "Search just in all opened files"},
    {CTRL_CHECK,                        "Without &prompt for replace",  ID_FF_PROMPT,   &FindDialog::s_vars.noPrompt,       20, 17,  0,  0, "Replace in all files without prompt"},

    {CTRL_STATIC,                       "&Code page:",                  0,              {},                                 54, 13, 14},
    {CTRL_DROPLIST,                     "",                             ID_FF_CP,       &FileDialog::s_vars.cp,             54, 14, 13,  6, "Select file code page"}
};

FindFileDialog::FindFileDialog(bool replace, pos_t x, pos_t y)
    : Dialog(findFileDialog, x, y)
    , m_replace{ replace }
{
}

bool FindFileDialog::OnActivate()
{
    Wnd* wnd = WndManager::getInstance().GetWnd();
    if (wnd && wnd->GetWndType() == wnd_t::editor)
    {
        auto editor = dynamic_cast<EditorWnd*>(wnd);

        std::u16string curWord;
        bool cur = editor->GetWord(curWord);
        auto word = utf8::utf16to8(curWord);

        auto seach = GetItem(ID_FF_SEARCH);
        auto ctrlSearch = std::dynamic_pointer_cast<CtrlEditDropList>(seach);
        if (ctrlSearch)
        {
            if (cur)
                ctrlSearch->AppendStr(word);
            for (auto& str : FindDialog::s_vars.findList)
                if (str != word)
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
                for (auto& str : FindDialog::s_vars.replaceList)
                    if (str != word)
                        ctrlSearch->AppendStr(str);
            }
        }
    }

    if (!m_replace)
    {
        GetItem(0)->SetName("Find");
        GetItem(ID_OK)->SetName("Find");

        GetItem(ID_FF_SREPLACE)->SetMode(CTRL_HIDE);
        GetItem(ID_FF_REPLACE)->SetMode(CTRL_HIDE);
        GetItem(ID_FF_PROMPT)->SetMode(CTRL_HIDE);

        pos_t x, y, sizex, sizey;
        auto smask = GetItem(ID_FF_SMASK);
        smask->GetPos(x, y, sizex, sizey);
        smask->SetPos(x, y - 1);

        auto mask = GetItem(ID_FF_MASK);
        mask->GetPos(x, y, sizex, sizey);
        mask->SetPos(x, y - 1);
    }
    else
    {
        GetItem(0)->SetName("Find And Repalece");
        GetItem(ID_OK)->SetName("Replace");
    }

    auto cp = GetItem(ID_FF_CP);
    auto ctrlCp = std::dynamic_pointer_cast<CtrlDropList>(cp);
    if (ctrlCp)
    {
        for (const std::string& str : iconvpp::CpConverter::GetCpList())
            ctrlCp->AppendStr(str);
        ctrlCp->SetSelect(FileDialog::s_vars.cp);
    }

    auto name = GetItem(ID_FF_MASK);
    auto ctrlName = std::dynamic_pointer_cast<CtrlEditDropList>(name);
    if (FileDialog::s_vars.maskList.empty())
        FileDialog::s_vars.maskList.push_front("*.*");
    for (const std::string& str : FileDialog::s_vars.maskList)
        ctrlName->AppendStr(str);

    m_dirList.SetMask(FileDialog::s_vars.maskList.front());
    ScanDir(FileDialog::s_vars.path);

    return true;
}

bool FindFileDialog::ScanDir(const std::string& mask)
{
    std::u16string wmask = utf8::utf8to16(mask);
    m_dirList.SetMask(wmask);
    m_dirList.Scan();

    auto path = m_dirList.GetPath() / utf8::utf8to16(m_dirList.GetMask());
    auto pathCtrl = GetItem(ID_FF_PATH);
    pathCtrl->SetName(Directory::CutPath(path, pathCtrl->GetSizeX()));

    auto fList = GetItem(ID_FF_FILELIST);
    auto fListPtr = std::dynamic_pointer_cast<CtrlList>(fList);
    fListPtr->Clear();

    for (auto& file : m_dirList.GetFileList())
        fListPtr->AppendStr(file.path().filename().u8string());

    auto dList = GetItem(ID_FF_DIRLIST);
    auto dListPtr = std::dynamic_pointer_cast<CtrlList>(dList);
    dListPtr->Clear();

    for (auto& dir : m_dirList.GetDirList())
        dListPtr->AppendStr(dir);
    for (auto& drv : m_dirList.GetDrvList())
        dListPtr->AppendStr(drv);

    int item = SelectItem(ID_FF_FILELIST);
    DialogProc(K_SELECT);

    SelectItem(item);
    Refresh();

    return m_dirList.IsFound();
}

input_t FindFileDialog::DialogProc(input_t code)
{
    //LOG(DEBUG) << __FUNC__ << " code=" << std::hex << code << std::dec;

    if ((code & K_TYPEMASK) == K_SYMBOL)
    {
        if (code == K_ENTER)
        {
            if (GetSelectedId() == ID_FF_DIRLIST)
            {
                auto list = GetItem(ID_FF_DIRLIST);
                auto listPtr = std::dynamic_pointer_cast<CtrlList>(list);
                auto item = listPtr->GetSelected();
                auto dir = listPtr->GetStr(item);

                ScanDir(std::string(dir));
                code = 0;
            }
        }
        else if (code == K_BS)
        {
            if (GetSelectedId() == ID_FF_DIRLIST)
            {
                //LOG(DEBUG) << "Dirlist BS";
                ScanDir("..");
                code = 0;
            }
        }
    }

    return code;
}

bool FindFileDialog::OnClose(int id)
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
                _TRY(FindDialog::s_vars.findList.emplace(str))
                FindDialog::s_vars.findStrW = utf8::utf8to16(str);

            size_t n = ctrlSearch->GetStrCount();
            FindDialog::s_vars.findList.clear();
            for (size_t i = 0; i < n && i < 16; ++i)
                _TRY(FindDialog::s_vars.findList.emplace(ctrlSearch->GetStr(i)))
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
                    _TRY(FindDialog::s_vars.replaceList.emplace(str))
                    FindDialog::s_vars.replaceStrW = utf8::utf8to16(str);

                size_t n = ctrlReplace->GetStrCount();
                FindDialog::s_vars.replaceList.clear();
                for (size_t i = 0; i < n && i < 16; ++i)
                    _TRY(FindDialog::s_vars.replaceList.emplace(ctrlReplace->GetStr(i)))
            }
        }

        auto path = m_dirList.GetPath();
        auto mask = GetItem(ID_FF_MASK)->GetName();
        LOG(DEBUG) << "path=" << path.u8string() << " mask=" << mask;

        FileDialog::s_vars.path = path.u8string();
        FileDialog::s_vars.cpName = GetItem(ID_FF_CP)->GetName();

        FileDialog::s_vars.maskList.remove(mask);
        FileDialog::s_vars.maskList.push_front(mask);
        if (FileDialog::s_vars.maskList.size() > MAX_MASK_LIST)
            FileDialog::s_vars.maskList.pop_back();

    }
    return true;
}


/////////////////////////////////////////////////////////////////////////////
std::vector<path_t> SearchFileDialog::s_foundList;
size_t SearchFileDialog::s_listPos{};

#define ID_SF_PATH     (ID_USER +  1)
#define ID_SF_FILELIST (ID_USER +  2)
#define ID_SF_PROGRESS (ID_USER +  3)
#define ID_SF_COUNT    (ID_USER +  4)

std::list<control> dlgSearchFile
{
    {CTRL_TITLE,                    "File Search",  0,              {},  1,  0, 70, 21},

    {CTRL_STATIC,                   "In:",          0,              {},  1,  0,  4},
    {CTRL_STATIC,                   "",             ID_SF_PATH,     {},  5,  0, 62},

    {CTRL_LIST,                     "",             ID_SF_FILELIST, {},  0,  1, 68, 17},
    {CTRL_STATIC,                   "",             ID_SF_PROGRESS, {},  1, 18,  1},
    {CTRL_STATIC,                   "",             ID_SF_COUNT,    {},  3, 18, 35},
    {CTRL_BUTTON | CTRL_ALIGN_RIGHT,"Stop",         ID_CANCEL,      {}, 40, 18}
};

SearchFileDialog::SearchFileDialog(pos_t x, pos_t y)
    : Dialog(dlgSearchFile, x, y)
{
}

input_t SearchFileDialog::Activate()
{
    path_t path{ utf8::utf8to16(FileDialog::s_vars.path) };
    LOG(DEBUG) << "SearchFile path=" << path << " mask=" << m_mask << " to find=" << utf8::utf16to8(m_toFind) << " cp=" << m_cp;

    m_activeView = WndManager::getInstance().GetActiveView();

    AllignButtons();
    Show();
    Refresh();
    InputCapture();

    Application::getInstance().SetErrorLine("Wait for file scan. Press any key for stop");
    s_foundList.clear();
    s_listPos = 0;
    [[maybe_unused]]auto rc = ScanDir(path / utf8::utf8to16(m_mask));

    Hide();
    WndManager::getInstance().SetActiveView(m_activeView);
    Application::getInstance().SetHelpLine();

    return ID_OK;
}

bool SearchFileDialog::ScanDir(const path_t& path)
{
    //LOG(DEBUG) << __FUNC__ << "path=" << path.u8string();
    
    auto sizex = GetItem(ID_SF_PATH)->GetSizeX();
    //std::string shortPath = Directory::CutPath(path, sizex);
    //shortPath.resize(sizex, ' ');
    //GetItem(ID_SF_PATH)->SetName(shortPath);

    _Utils::DirectoryList dirList;

    dirList.SetMask(path);
    dirList.SetMask(m_mask);
    dirList.Scan();
    
    bool putPath{};
    auto fList = GetItem(ID_SF_FILELIST);
    auto fListPtr = std::dynamic_pointer_cast<CtrlList>(fList);

    for (auto& file : dirList.GetFileList())
    {
        if(!ShowProgress())
            return false;

        if (FindFileDialog::s_vars.inOpen)
        {
            auto& app = Application::getInstance();
            auto& editorApp = dynamic_cast<EditorApp&>(app);
            auto wnd = editorApp.GetEditorWnd(file.path());
            if (nullptr == wnd)
                continue;
        }
        
        std::string shortPath = Directory::CutPath(file.path(), sizex);
        shortPath.resize(sizex, ' ');
        GetItem(ID_SF_PATH)->SetName(shortPath);

        bool ret = Editor::ScanFile(file.path(), m_toFind, m_cp, m_checkCase, m_findWord, std::bind(&SearchFileDialog::ShowProgress, this));
        if (ret)
        {
            //found
            LOG(DEBUG) << "found in file=" << file.path().u8string();
            s_foundList.emplace_back(file.path());
            std::string count = "Matched " + std::to_string(s_foundList.size()) + " file(s).";
            GetItem(ID_SF_COUNT)->SetName(count);

            if (!putPath)
            {
                putPath = true;
                fListPtr->AppendStr(file.path().parent_path().u8string());
            }
            fListPtr->AppendStr("  " + file.path().filename().u8string());
            fListPtr->SetSelect(fListPtr->GetStrCount() - 1);
        }
    }

    auto key = CheckInput();
    if (key)
        return false;

    if(m_recursive)
        for (auto& dir : dirList.GetDirList())
        {
            if (dir == "..")
                continue;
            bool rc = ScanDir(dirList.GetPath() / utf8::utf8to16(dir));
            if (!rc)
                return false;
        }

    return true;
}

bool SearchFileDialog::ShowProgress()
{
    if (m_cancel)
        return false;

    char buff[2]{ s_progress[m_pos], 0 };
    if (++m_pos >= s_progress.size())
        m_pos = 0;

    GetItem(ID_SF_PROGRESS)->SetName(buff);

    auto key = CheckInput();
    if (key)
    {
        m_cancel = true;
        return false;
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
path_t MatchedFileDialog::s_path;

#define ID_MF_COUNT    (ID_USER +  1)
#define ID_MF_FILELIST (ID_USER +  2)

std::list<control> dlgMatchedFile 
{
    {CTRL_TITLE,                        "Matched Files",    0,              {},                             1,  0, 70, 21},

    {CTRL_LIST,                         "",                 ID_MF_FILELIST, &SearchFileDialog::s_listPos,   0,  0, 68, 18, "Select file for open"},
    {CTRL_STATIC,                       "",                 ID_MF_COUNT,    {},                             1, 18, 47},

    {CTRL_DEFBUTTON | CTRL_ALIGN_RIGHT, "Open",             ID_OK,          {},                            40, 18,  0,  0, "Open selected file in new windows"},
    {CTRL_BUTTON | CTRL_ALIGN_RIGHT,    "Cancel",           ID_CANCEL,      {},                            50, 18}
};

MatchedFileDialog::MatchedFileDialog(pos_t x, pos_t y)
    : Dialog(dlgMatchedFile, x, y)
{
}

bool MatchedFileDialog::OnActivate()
{
    if(SearchFileDialog::s_foundList.empty())
    {
        BeginPaint();
        Application::getInstance().SetErrorLine("Matched files absent");
        StopPaint();
        return false;
    }

    auto fList = GetItem(ID_MF_FILELIST);
    auto fListPtr = std::dynamic_pointer_cast<CtrlList>(fList);
    path_t curPath;

    for (auto& fullPath : SearchFileDialog::s_foundList)
    {
        auto path = fullPath.parent_path();
        if (curPath != path)
        {
            if(curPath != "")
                fListPtr->AppendStr("");

            curPath = path;
            fListPtr->AppendStr(path.u8string());
        }
        fListPtr->AppendStr("  " + fullPath.filename().u8string());
    }

    fListPtr->SetSelect(SearchFileDialog::s_listPos);
    GetItem(ID_MF_COUNT)->SetName("Found " + std::to_string(SearchFileDialog::s_foundList.size()) + " file(s).");

    return true;
}

bool MatchedFileDialog::OnClose(int id)
{
    if (id == ID_OK)
    {
        auto fList = GetItem(ID_MF_FILELIST);
        auto fListPtr = std::dynamic_pointer_cast<CtrlList>(fList);

        auto pos = fListPtr->GetSelected();
        auto str = fListPtr->GetStr(pos);
        if (str.empty() || str[0] != ' ')
        {
            SelectItem(ID_MF_FILELIST);
            Refresh();
            return false;
        }

        size_t entry{};
        path_t curPath;
        for (auto& fullPath : SearchFileDialog::s_foundList)
        {
            auto path = fullPath.parent_path();
            if (curPath != path)
            {
                if (curPath != "")
                    ++entry;

                curPath = path;
                ++entry;
            }
            if (entry == pos)
            {
                s_path = fullPath;
                break;
            }
            ++entry;
        }

    }

    return true;
}

} //namespace _Editor
