/*
FreeBSD License

Copyright (c) 2020-2023 vikonix: valeriy.kovalev.software@gmail.com
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
#include "WndManager/App.h"
#include "EditorWnd.h"


namespace _Editor
{

extern Logo         g_logo;
extern sline_list   g_statusLine;
extern menu_list    g_accessMenu;
extern menu_list    g_replaceMenu;
extern menu_list    g_menuRecentFiles;
extern menu_list    g_menuRecentSessions;
extern menu_list    g_popupMenu;
extern std::vector<menu_list> g_mainMenu;

extern const size_t c_recentFilesMenu;
extern const size_t c_recentSessionsMenu;

using file_t = std::tuple<std::string, std::string, std::string, bool, bool>;

class EditorApp : public Application
{
    using AppFunc = std::function<bool(EditorApp*, input_t)>;
    static std::unordered_map<AppCmd, AppFunc> s_cmdMap;
    
    inline static const size_t c_maxRecentFiles{16};
    std::deque<file_t> m_recentFiles;
    std::unordered_map<Wnd*, std::shared_ptr<EditorWnd>> m_editors;

    bool m_wait{};
    bool m_run{};

    bool CloseAllWindows();
    bool UpdateRecentFilesList();

public:
    virtual input_t AppProc(input_t code) override final;
    virtual bool    LoadCfg() override final;
    virtual bool    SaveCfg(input_t code = 0) override final;
    virtual bool    LoadSession(std::optional<const std::filesystem::path> path) override final;
    virtual bool    SaveSession(std::optional<const std::filesystem::path> path) override final;

    virtual bool    Init() override final;
    virtual void    Deinit() override final;
    virtual bool    CloseWindow(Wnd* wnd) override final;
    virtual void    StatusWaitKey(bool wait) override final;
    virtual void    StatusRecordMacro(bool run) override final;

    virtual std::string GetKeyName(input_t code) const override final;
    virtual std::string GetCodeName(input_t code) const override final;
    virtual input_t GetCode(const std::string& code) const override final;

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

    Wnd* GetEditorWnd(std::filesystem::path path);
    bool OpenFile(const std::filesystem::path& path, const std::string& parseMode, const std::string& cp, bool ro = false, bool log = false);

    //editor app commands
    bool    AboutCmd(input_t cmd);
    bool    HelpCmd(input_t cmd);
    bool    HelpKeymapCmd(input_t cmd);
    bool    FileNewCmd(input_t cmd);
    bool    FileSaveAllCmd(input_t cmd);
    bool    FileOpenCmd(input_t cmd);
    bool    WndCloseAllCmd(input_t cmd);
    bool    WndListCmd(input_t cmd);
    bool    FindInFilesCmd(input_t cmd);
    bool    ReplaceInFilesCmd(input_t cmd);
    bool    FoundFilesCmd(input_t cmd);
    bool    WndCopyCmd(input_t cmd);
    bool    WndMoveCmd(input_t cmd);
    bool    ViewSplitCmd(input_t cmd);
    bool    ViewModeCmd(input_t cmd);
    bool    ViewSelectCmd(input_t cmd);
    bool    ViewMoveCmd(input_t cmd);
    bool    DiffCmd(input_t cmd);
    bool    BookmarkListCmd(input_t cmd);
    bool    KeygenCmd(input_t cmd);
    bool    NewSessionCmd(input_t cmd);
    bool    OpenSessionCmd(input_t cmd);
    bool    RecordMacroCmd(input_t cmd);
    bool    PlayMacroCmd(input_t cmd);
    bool    ColorDlgCmd(input_t cmd);
    bool    SettingsDlgCmd(input_t cmd);

    bool    SelectBookmarkCmd(input_t cmd);
    bool    SelectRecentFileCmd(input_t cmd);
    bool    SelectRecentSessionCmd(input_t cmd);
};

} //namespace _Editor
