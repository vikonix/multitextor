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
#include "Dialog.h"
#include "utils/Directory.h"


enum class FileDlgMode
{
    Open,
    Save,
    NewSess,
    OpenSess
};

#define MAX_MASK_LIST 16
struct FileDialogVars
{
    std::list<std::string> typeList{ "Text", "C++" };
    std::list<std::string> cpList{ "UTF-8", "CP437", "CP866", "CP1251" };
    std::list<std::string> maskList;

    std::string path{"."};
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
    FileDlgMode     m_mode;
    DirectoryList   m_dirList;

    bool ScanDir(const std::string& mask);

public:
    static FileDialogVars s_vars;

    FileDialog(FileDlgMode mode = FileDlgMode::Open, pos_t x = MAX_COORD, pos_t y = MAX_COORD);

    virtual input_t DialogProc(input_t code) override;
    virtual bool OnActivate() override;
    virtual bool OnClose(int id) override;
};
