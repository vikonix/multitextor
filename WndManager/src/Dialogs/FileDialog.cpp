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
#include "Dialogs/StdDialogs.h"
#include "DlgControls.h"
#include "WndManager.h"


/////////////////////////////////////////////////////////////////////////////
#define ID_OF_NAME      (ID_USER + 1)
#define ID_OF_PATH      (ID_USER + 2)
#define ID_OF_FILELIST  (ID_USER + 3)
#define ID_OF_DIRLIST   (ID_USER + 4)
#define ID_OF_STAT_CP   (ID_USER + 5)
#define ID_OF_CP        (ID_USER + 6)
#define ID_OF_STAT_TYPE (ID_USER + 7)
#define ID_OF_TYPE      (ID_USER + 8)
#define ID_OF_RO        (ID_USER + 9)
#define ID_OF_LOG       (ID_USER + 10)
#define ID_OF_INFO      (ID_USER + 11)

FileDialogVars FileDialog::s_vars;

std::list<control> fileDialog 
{
    {CTRL_TITLE,                        "",             0,              nullptr,                    1,  0, 70, 21},

    {CTRL_STATIC,                       "File &name:",  0,              nullptr,                    1,  1, 14},
    {CTRL_EDITDROPLIST,                 "",             ID_OF_NAME,     &FileDialog::s_vars.file,  15,  1, 52,  7, "Input file name or mask"},

    {CTRL_STATIC | CTRL_NOCOLOR,        "",             ID_OF_PATH,     nullptr,                    1,  3, 66},

    {CTRL_LIST,                         "&Directories", ID_OF_DIRLIST,  (size_t*)nullptr,           0,  4, 19, 14, "Select directory"},
    {CTRL_LIST,                         "&Files",       ID_OF_FILELIST, (size_t*)nullptr,          19,  4, 34, 14, "Select file name"},

    {CTRL_DEFBUTTON | CTRL_ALIGN_LEFT,  "",             ID_OK,          nullptr,                   54,  5},
    {CTRL_BUTTON | CTRL_ALIGN_LEFT,     "Cancel",       ID_CANCEL,      nullptr,                   54,  7},

    {CTRL_STATIC,                       "File &type:",  ID_OF_STAT_TYPE,nullptr,                   54,  9, 14},
    {CTRL_DROPLIST,                     "",             ID_OF_TYPE,     &FileDialog::s_vars.type,  54, 10, 13,  6, "Select file type"},
    {CTRL_STATIC,                       "Code &page:",  ID_OF_STAT_CP,  nullptr,                   54, 12, 14},
    {CTRL_DROPLIST,                     "",             ID_OF_CP,       &FileDialog::s_vars.cp,    54, 13, 13,  6, "Select file code page"},

    {CTRL_CHECK,                        "&Read only",   ID_OF_RO,       &FileDialog::s_vars.ro,    54, 15,  0,  0, "Open file as read only"},
    {CTRL_CHECK,                        "&Log file",    ID_OF_LOG,      &FileDialog::s_vars.log,   54, 16,  0,  0, "Open file that can grow"},
    {CTRL_STATIC,                       "",             ID_OF_INFO,     nullptr,                   20, 18, 34},
    {CTRL_LINE,                         "",             0,              nullptr,                   54, 17, 13}
};

FileDialog::FileDialog(FileDlgMode mode, pos_t x, pos_t y)
    : Dialog(fileDialog, x, y)
    , m_mode{ mode }
{
}

bool FileDialog::OnActivate()
{
    bool hide{false};

    switch (m_mode)
    {
    case FileDlgMode::Open:
        GetItem(0)->SetName("Open File");
        GetItem(ID_OK)->SetName("Open");
        GetItem(ID_OK)->SetHelpLine("Open file in new place");
        break;
    case FileDlgMode::SaveAs:
        {
        GetItem(0)->SetName("Save File As");
        GetItem(ID_OK)->SetName("Save");
        GetItem(ID_OK)->SetHelpLine("Save file with new name");
        hide = true;

        Wnd* wnd = WndManager::getInstance().GetWnd();
        auto path = wnd->GetFilePath();

        auto name = GetItem(ID_OF_NAME);
        auto ctrlName = std::dynamic_pointer_cast<CtrlEditDropList>(name);
        ctrlName->SetName(path.filename().u8string());
        s_vars.path = path.parent_path().u8string();
        }
        break;
    default: //???
        return false;
    }

    if (hide)
    {
        GetItem(ID_OF_STAT_TYPE)->SetMode(CTRL_HIDE);
        GetItem(ID_OF_TYPE)->SetMode(CTRL_HIDE);
        GetItem(ID_OF_STAT_CP)->SetMode(CTRL_HIDE);
        GetItem(ID_OF_CP)->SetMode(CTRL_HIDE);
        GetItem(ID_OF_RO)->SetMode(CTRL_HIDE);
        GetItem(ID_OF_LOG)->SetMode(CTRL_HIDE);
    }
    else
    {
        auto cp = GetItem(ID_OF_CP);
        auto ctrlCp = std::dynamic_pointer_cast<CtrlDropList>(cp);
        if (ctrlCp)
        {
            for (const std::string& str : s_vars.cpList)
                ctrlCp->AppendStr(str);
            ctrlCp->SetSelect(s_vars.cp);
        }

        auto type = GetItem(ID_OF_TYPE);
        auto ctrlType = std::dynamic_pointer_cast<CtrlDropList>(type);
        if (ctrlType)
        {
            for (const std::string& str : s_vars.typeList)
                ctrlType->AppendStr(str);
            ctrlType->SetSelect(s_vars.type);
        }
    }

    auto name = GetItem(ID_OF_NAME);
    auto ctrlName = std::dynamic_pointer_cast<CtrlEditDropList>(name);
    if (s_vars.maskList.empty())
        s_vars.maskList.push_front("*.*");
    for (const std::string& str : s_vars.maskList)
        ctrlName->AppendStr(str);

    m_dirList.SetMask(s_vars.maskList.front());
    ScanDir(s_vars.path);

    return true;
}

bool FileDialog::ScanDir(const std::string& mask)
{
    std::u16string wmask = utf8::utf8to16(mask);
    m_dirList.SetMask(wmask);
    m_dirList.Scan();

    if (m_mode != FileDlgMode::SaveAs && m_mode != FileDlgMode::NewSess)
        GetItem(ID_OF_NAME)->SetName("");
    GetItem(ID_OF_INFO)->SetName("");

    auto path = m_dirList.GetPath() / utf8::utf8to16(m_dirList.GetMask());
    auto pathCtrl = GetItem(ID_OF_PATH);
    pathCtrl->SetName(Directory::CutPath(path, pathCtrl->GetSizeX()));
    
    auto fList = GetItem(ID_OF_FILELIST);
    auto fListPtr = std::dynamic_pointer_cast<CtrlList>(fList);
    fListPtr->Clear();

    for (auto& file : m_dirList.GetFileList())
        fListPtr->AppendStr(file.path().filename().u8string());

    auto dList = GetItem(ID_OF_DIRLIST);
    auto dListPtr = std::dynamic_pointer_cast<CtrlList>(dList);
    dListPtr->Clear();

    for (auto& dir : m_dirList.GetDirList())
        dListPtr->AppendStr(dir);
    for (auto& drv : m_dirList.GetDrvList())
        dListPtr->AppendStr(drv);

    int item = SelectItem(ID_OF_FILELIST);
    DialogProc(K_SELECT);

    SelectItem(item);
    Refresh();

    return m_dirList.IsFound();
}

input_t FileDialog::DialogProc(input_t code)
{
    //LOG(DEBUG) << __FUNC__ << " code=" << std::hex << code << std::dec;

    if ((code & K_TYPEMASK) == K_SELECT)
    {
        if (GetSelectedId() == ID_OF_FILELIST)
        {
            //file list selected
            auto item = K_GET_CODE(code);
            auto& list = m_dirList.GetFileList();
            if (item >= list.size())
                return code;
            auto& info = list[item];
            auto ftime = info.last_write_time();
            std::time_t cftime = to_time_t(ftime);
            std::tm tm = *std::localtime(&cftime);

            std::stringstream sinfo;
            sinfo << std::put_time(&tm, "%d %b %Y %H:%M:%S");

            auto size = info.file_size();
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

            GetItem(ID_OF_INFO)->SetName(sinfo.str());

            if (m_mode != FileDlgMode::SaveAs)
            {
                GetItem(ID_OF_NAME)->SetName(info.path().filename().u8string());
                /*???
                if (m_nMode != DIALOG_OPENSESS && m_nMode != DIALOG_CMD)// && m_nMode != DIALOG_KEY)
                {
                    CtrlSList* pCtrl = (CtrlSList*)GetItem(ID_OF_TYPE);
                    pCtrl->SetSelect(g_LexCfg.CheckFile(pInfo->sName));
                }
                */
            }
        }
        else if (GetSelectedId() == ID_OF_DIRLIST)
        {
            //dir list selected
            if (m_mode != FileDlgMode::SaveAs)
            {
                size_t item = K_GET_CODE(code);
                auto list = GetItem(ID_OF_DIRLIST);
                auto listPtr = std::dynamic_pointer_cast<CtrlList>(list);
                auto dir = listPtr->GetStr(item);

                GetItem(ID_OF_NAME)->SetName(std::string(dir));
            }
        }
    }
    else if ((code & K_TYPEMASK) == K_SYMBOL)
    {
        if (code == K_ENTER)
        {
            if (GetSelectedId() == ID_OF_DIRLIST)
            {
                auto list = GetItem(ID_OF_DIRLIST);
                auto listPtr = std::dynamic_pointer_cast<CtrlList>(list);
                auto item = listPtr->GetSelected();
                auto dir = listPtr->GetStr(item);
                
                ScanDir(std::string(dir));
                code = 0;
            }
            else if (GetSelectedId() == ID_OF_FILELIST)
            {
                auto list = GetItem(ID_OF_FILELIST);
                auto listPtr = std::dynamic_pointer_cast<CtrlList>(list);
                auto item = listPtr->GetSelected();
                auto file = listPtr->GetStr(item);

                auto edit = GetItem(ID_OF_NAME);
                auto editPtr = std::dynamic_pointer_cast<CtrlEditDropList>(edit);
                editPtr->SetName(std::string(file));
            }
            else
            {
                auto name = GetItem(ID_OF_NAME)->GetName();
                auto found = ScanDir(std::string(name));

                if (!found && !m_dirList.IsSingleMask())
                {
                    //if not simple mask or found many files
                    auto mask = m_dirList.GetMask();
                    LOG(DEBUG) << "Mask " << mask;

                    s_vars.maskList.remove(mask);
                    s_vars.maskList.push_front(mask);
                    if (s_vars.maskList.size() > MAX_MASK_LIST)
                        s_vars.maskList.pop_back();

                    auto type = GetItem(ID_OF_NAME);
                    auto ctrlType = std::dynamic_pointer_cast<CtrlEditDropList>(type);
                    ctrlType->Clear();
                    for (const std::string& str : s_vars.maskList)
                        ctrlType->AppendStr(str);

                    code = 0;
                }
                else
                {
                    LOG(DEBUG) << "Found";
                }
            }
        }
        else if (code == K_BS)
        {
            if (GetSelectedId() == ID_OF_DIRLIST)
            {
                LOG(DEBUG) << "Dirlist BS";
                ScanDir("..");
                code = 0;
            }
        }
    }

    return code;
}

bool FileDialog::OnClose(int id)
{
    if (id == ID_OK)
    {
        auto path = m_dirList.GetPath();
        auto name = GetItem(ID_OF_NAME)->GetName();
        LOG(DEBUG) << "path=" << path.u8string() << " file=" << name;
        s_vars.path = path.u8string();
        s_vars.cpName = GetItem(ID_OF_CP)->GetName();
        s_vars.typeName = GetItem(ID_OF_TYPE)->GetName();
    }
    return true;
}