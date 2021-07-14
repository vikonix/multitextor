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
#pragma once
#include "WndManager/Dialog.h"
#include "utils/Directory.h"

#include <map>
#include <set>

using namespace _Utils;
using namespace _WndManager;

namespace _Editor
{

/////////////////////////////////////////////////////////////////////////////
enum class FileDlgMode
{
    Open,
    SaveAs,
    NewSess,
    OpenSess
};

#define MAX_MASK_LIST 16
struct FileDialogVars
{
    path_t      filepath;

    std::list<std::string> maskList;

    std::string path{ "." };
    std::string file{ "*.*" };
    std::string typeName;
    std::string cpName;

    size_t type{};
    size_t cp{};
    bool ro{};
    bool log{};
};

class FileDialog : public Dialog
{
    FileDlgMode             m_mode;
    _Utils::DirectoryList   m_dirList;

    bool ScanDir(const std::string& mask);

public:
    static FileDialogVars s_vars;

    FileDialog(FileDlgMode mode = FileDlgMode::Open, pos_t x = MAX_COORD, pos_t y = MAX_COORD);

    virtual input_t DialogProc(input_t code) override final;
    virtual bool OnActivate() override final;
    virtual bool OnClose(int id) override final;
};
    
/////////////////////////////////////////////////////////////////////////////
enum class WindowsDlgMode
{
    List,
    CopyFrom,
    MoveFrom,
    CompareWith
};

class EditorWnd;
class WindowListDialog : public Dialog
{
    WindowsDlgMode m_mode;
    std::map<std::string, EditorWnd*> m_wndList;

    size_t GetWndList(bool skip = false);

public:
    WindowListDialog(WindowsDlgMode mode = WindowsDlgMode::List, pos_t x = MAX_COORD, pos_t y = MAX_COORD);

    virtual input_t DialogProc(input_t code) override final;
    virtual bool OnActivate() override final;
    virtual bool OnClose(int id) override final;
};

/////////////////////////////////////////////////////////////////////////////
class GotoDialog : public Dialog
{
    size_t m_maxLine;

public:
    static std::string m_line;

    GotoDialog(size_t maxLine, pos_t x = MAX_COORD, pos_t y = MAX_COORD);
    virtual bool OnClose(int id) override final;

    size_t GetLine() { return static_cast<size_t>(std::stoull(m_line)); }
};

/////////////////////////////////////////////////////////////////////////////
struct FindReplaceVars
{
    std::set<std::string>  findList;
    std::set<std::string>  replaceList;

    std::string     findStr;
    std::string     replaceStr;
    std::u16string  findStrW;
    std::u16string  replaceStrW;

    bool            checkCase{};
    bool            directionUp{};
    bool            inSelected{};
    bool            findWord{};

    bool            replaceMode{};
    bool            noPrompt{};
};

class FindDialog : public Dialog
{
    bool m_replace;//false-find true-replace

public:
    static FindReplaceVars s_vars;

    FindDialog(bool replace, pos_t x = MAX_COORD, pos_t y = MAX_COORD);

    virtual bool OnActivate() override final;
    virtual bool OnClose(int id) override final;
};

/////////////////////////////////////////////////////////////////////////////
struct PropertiesVars
{
    //eol_t same order
    std::list<std::string> eolList{ "Unix (LF)", "Windows (CR+LF)", "Mac (CR)" };

    size_t type{};
    size_t cp{};

    std::string typeName;
    std::string cpName;
    size_t eol{};
    size_t tabSize{ 4 };
    size_t saveTab{};
    bool showTab{};
    bool ro{};
    bool log{};
    bool untitled{};
};

class PropertiesDialog : public Dialog
{
public:
    static PropertiesVars s_vars;

    PropertiesDialog(pos_t x = MAX_COORD, pos_t y = MAX_COORD);

    virtual bool OnActivate() override final;
    virtual bool OnClose(int id) override final;
};

/////////////////////////////////////////////////////////////////////////////
struct FindFileDialogVars
{
    bool            inOpen{};
    bool            recursive{true};
};

class FindFileDialog : public Dialog
{
    bool m_replace;//false-find true-replace

    _Utils::DirectoryList   m_dirList;

    bool ScanDir(const std::string& mask);

public:
    static FindFileDialogVars s_vars;

    FindFileDialog(bool replace, pos_t x = MAX_COORD, pos_t y = MAX_COORD);

    virtual input_t DialogProc(input_t code) override final;
    virtual bool OnActivate() override final;
    virtual bool OnClose(int id) override final;
};

/////////////////////////////////////////////////////////////////////////////
class SearchFileDialog : public Dialog
{
    inline static const std::string s_progress{"-\\|/"};
    size_t              m_pos{};
    bool                m_cancel{};

    bool ScanDir(const path_t& path);
    bool ShowProgress();

    std::string&    m_mask{ FileDialog::s_vars.file };
    std::u16string& m_toFind{ FindDialog::s_vars.findStrW };
    std::string&    m_cp{ FileDialog::s_vars.cpName };
    bool            m_recursive{ FindFileDialog::s_vars.recursive };
    bool            m_checkCase{ FindDialog::s_vars.checkCase };
    bool            m_findWord{ FindDialog::s_vars.findWord };

public:
    static std::vector<path_t>  s_foundList;
    static size_t               s_listPos;

    SearchFileDialog(pos_t x = MAX_COORD, pos_t y = MAX_COORD);

    virtual input_t Activate() override final;
};

class MatchedFileDialog : public Dialog
{
public:
    static path_t s_path;

    MatchedFileDialog(pos_t x = MAX_COORD, pos_t y = MAX_COORD);

    virtual bool OnActivate() override final;
    virtual bool OnClose(int id) override final;
};

} //namespace _Editor
