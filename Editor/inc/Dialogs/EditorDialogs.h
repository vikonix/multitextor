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

#include <map>

using namespace _WndManager;

namespace _Editor
{

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

    virtual input_t DialogProc(input_t code) override;
    virtual bool OnActivate() override;
    virtual bool OnClose(int id) override;
};

class GotoDialog : public Dialog
{
    size_t m_maxLine;

public:
    static std::string m_line;

    GotoDialog(size_t maxLine, pos_t x = MAX_COORD, pos_t y = MAX_COORD);
    virtual bool OnClose(int id) override final;

    size_t GetLine() { return std::stoull(m_line); }
};

struct FindReplaceVars
{
    std::list<std::string>  findList;
    std::list<std::string>  replaceList;

    std::string     findStr;
    std::string     replaceStr;
    std::u16string  findStrW;
    std::u16string  replaceStrW;

    bool            checkCase{};
    bool            directionUp{};
    bool            replaceMode{};
    bool            inSelected{};
    bool            findWord{};
    bool            noPrompt{};
};

class FindDialog : public Dialog
{
    bool m_replace;//false-find true-replace

public:
    static FindReplaceVars s_vars;

    FindDialog(bool replace, pos_t x = MAX_COORD, pos_t y = MAX_COORD);

    bool OnActivate();
    bool OnClose(int id);
};

struct PropertiesVars
{
    std::list<std::string> typeList{ "Text", "C++" };
    std::list<std::string> cpList{ "UTF-8", "CP437", "CP866", "CP1251" };
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
};

class PropertiesDialog : public Dialog
{
public:
    static PropertiesVars s_vars;

    PropertiesDialog(pos_t x = MAX_COORD, pos_t y = MAX_COORD);

    bool OnActivate();
    bool OnClose(int id);
};

} //namespace _Editor
