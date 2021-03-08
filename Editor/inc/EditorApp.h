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
#include "App.h"
#include "EditorWnd.h"

class EditorApp : public Application
{
    std::unordered_map<std::string, std::shared_ptr<EditorWnd>> m_editors;
    bool s_wait{};

public:
    virtual input_t AppProc(input_t code) override final;
    virtual bool    LoadCfg() override final;
    virtual bool    SaveCfg(input_t code = 0) override final;

    virtual bool    Init() override final;
    virtual void    Deinit() override final;
    virtual bool    StatusWaitKey(bool wait) override final;

    static bool SetHelpLine(std::optional<const std::string> help = std::nullopt, stat_color color = stat_color::normal)
    {
        return getInstance().ChangeStatusLine(0, help, color);
    }

    static bool SetErrorLine(const std::string& error)
    {
        return getInstance().ChangeStatusLine(0, error, stat_color::error);
    }

    static bool ShowProgressBar(size_t n = 100)
    {
        return getInstance().ShowProgressBar(n);
    }

    enum class mark_status
    {
        no,
        mark,
        mark_by_key
    };

    static bool StatusMark(mark_status mark = mark_status::no);
};
