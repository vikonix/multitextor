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
#include "EditorApp.h"
#include "Dialogs/EditorDialogs.h"
#include "Version.h"
#include "Config.h"
#include "utfcpp/utf8.h"


namespace _Editor
{

sline_list g_statusLine
{
    {"",     "",     stat_color::normal},//0
    {"Key",  "",     stat_color::grayed},//1
    {"Mark", "",     stat_color::grayed},//2
    {"Rec",  "Play", stat_color::grayed},//3
    {"Ins",  "Ovr",  stat_color::normal} //4
};

bool EditorApp::Init()
{
    Application::Init();
    SetStatusLine(g_statusLine);
    return true;
}

void EditorApp::Deinit()
{
    CloseAllWindows();
    Application::Deinit();
}

void EditorApp::StatusWaitKey(bool wait)
{
    if (m_wait != wait)
    {
        ChangeStatusLine(1, wait ? stat_color::normal : stat_color::grayed);
        if (wait)
            ChangeStatusLine(0, "Press next key of sequence");
        else
            ChangeStatusLine(0);
        m_wait = wait;
    }
}

void EditorApp::StatusRecordMacro(bool run)
{
    if (m_run != run)
    {
        ChangeStatusLine(3, run ? stat_color::normal : stat_color::grayed);
        if (run)
            ChangeStatusLine(0, "Press " + GetKeyName(K_APP_RECORD_MACRO) + " for end record macro");
        else
            ChangeStatusLine(0);
        m_run = run;
    }
}

bool EditorApp::StatusMark(mark_status mark)
{
    static mark_status s_mark{};
    if (s_mark != mark)
    {
        //this will clear all other help line
        getInstance().ChangeStatusLine(2, mark != mark_status::no ? stat_color::normal : stat_color::grayed);
        if (mark == mark_status::mark_by_key)
            getInstance().ChangeStatusLine(0, "Press " + getInstance().GetKeyName(K_ED(E_SELECT_END)) + " for to end selection");
        else
            getInstance().ChangeStatusLine(0);
        s_mark = mark;
    }

    return true;
}

std::string EditorApp::GetKeyName(input_t code) const
{
    std::vector<input_t> codeKeys;
    for (const auto& [keys, cmd] : g_defaultAppKeyMap)
    {
        if (cmd.size() == 1 && code == cmd[0])
        {
            codeKeys = keys;
            break;
        }
    }
    if(codeKeys.empty())
        for (const auto& [keys, cmd] : g_defaultEditKeyMap)
        {
            if (cmd.size() == 1 && code == cmd[0])
            {
                codeKeys = keys;
                break;
            }
        }
    if (codeKeys.empty())
        return {};

    std::string keyNames;
    for (const auto key : codeKeys)
    {
        if (!keyNames.empty())
            keyNames += " ";
        keyNames += GetCodeName(key);
    }

    return keyNames;
}

std::string EditorApp::GetCodeName(input_t code) const
{
    if (auto it = g_CmdNames.find(code); it != g_CmdNames.end())
    {
        return it->second;
    }

    std::string name;
    auto keyType = code & K_TYPEMASK;
    auto keyCmd = code & 0xffff0000;
    auto keyCode = K_GET_CODE(code);

    if (code & K_SHIFT)
        name += "Shift+";
    if (code & K_CTRL)
        name += "Ctrl+";
    if (code & K_ALT)
        name += "Alt+";

    bool found{};
    if (0 != keyType)
    {
        if (auto it = g_CmdNames.find(keyType); it != g_CmdNames.end())
        {
            name += it->second;
            found = true;
        }
    }
    if (!found && 0 != keyType && 0 != keyCmd)
    {
        if (auto it = g_CmdNames.find(keyCmd); it != g_CmdNames.end())
        {
            name += it->second;
            found = true;
        }
    }
    if (!found && 0 != keyCode)
    {
        if (auto it = g_CmdNames.find(keyCode); it != g_CmdNames.end())
        {
            name += it->second;
            found = true;
        }
    }
    if (!found && keyCode > ' ')
        name += static_cast<char>(keyCode);

    return name;
}

input_t EditorApp::GetCode(const std::string& code) const
{
    input_t retCode{};
    std::stringstream stream(code);

    std::string parsed;
    while (std::getline(stream, parsed, '+'))
    {
        bool found{};
        for (auto& [c, name] : g_CmdNames)
            if (parsed == name)
            {
                retCode |= c;
                found = true;
                break;
            }
        if (!found)
        {
            if (parsed.size() == 1)
                retCode |= parsed[0];
            else if(parsed.empty())
                retCode |= '+';
            else
            {
                _assert(0);
            }
        }

    }    
    
    return retCode;
}

bool EditorApp::CloseWindow(Wnd* wnd)
{
    if(auto it = m_editors.find(wnd); it != m_editors.end())
    {
        //LOG(DEBUG) << "CloseWindows " << wnd;
        m_editors.erase(it);
        return true;
    }

    return false;
}

Wnd* EditorApp::GetEditorWnd(std::filesystem::path path)
{
    for (auto& [ptr, wnd] : m_editors)
    {
        if (path == wnd->GetFilePath())
            return ptr;
    }
    return nullptr;
}

bool EditorApp::CloseAllWindows()
{
    for (auto& [ptr, wnd] : m_editors)
    {
        if (!wnd->IsChanged())
            continue;

        auto ret = MsgBox(MBoxKey::OK_CANCEL, "Close: " + wnd->GetObjectName(),
            { "File was changed.",
            "Do you want to save it?" },
            { "Save", "No" }
        );
        if (ret == ID_OK)
            wnd->Save(0);
    }

    m_editors.clear();
    return true;
}

input_t EditorApp::AppProc(input_t code)
{ 
    //input treatment in user function
    //LOG_IF(code != K_TIME, DEBUG) << __FUNC__ << " code=" << std::hex << code << std::dec;

    if (code == K_INSERT)
    {
        if (m_insert)
        {
            m_insert = false;
            SwapStatusLine(4);
        }
        else
        {
            m_insert = true;
            SwapStatusLine(4);
        }

        return 0;
    }
    else if (code == K_EXIT)
    {
        auto ret = MsgBox(MBoxKey::OK_CANCEL, "Exit",
            { "Do you want to exit program ?" }
        );
        if (ret != ID_OK)
            return 0;

        CloseAllWindows();
    }
    else if(code >= K_APP && code <= K_APP_END)
    {
        AppCmd cmd = static_cast<AppCmd>(code);
        if (code >= K_APP_BOOKMARK && code < K_APP_FILE_RECENT)
            cmd = K_APP_BOOKMARK;
        else if (code >= K_APP_FILE_RECENT && code < K_APP_SESSION_RECENT)
            cmd = K_APP_FILE_RECENT;
        else if (code > K_APP_SESSION_RECENT)
            cmd = K_APP_SESSION_RECENT;

        if (auto it = s_funcMap.find(cmd); it != s_funcMap.end())
        {
            auto& [k, func] = *it;
            [[maybe_unused]]bool rc = func(this, code);
            return 0;
        }
    }

    return code; 
} 

bool EditorApp::UpdateRecentFilesList()
{
    g_menuRecentFiles.clear();
    input_t n{};
    for (auto& entry : m_recentFiles)
        g_menuRecentFiles.emplace_back(menu{ MENU_ITEM, std::get<0>(entry), K_APP_FILE_RECENT + n++ });

    if (m_menuArray.size() >= c_recentFilesMenu)
        m_menuArray[c_recentFilesMenu] = g_menuRecentFiles;
    return true;
}

bool EditorApp::OpenFile(const std::filesystem::path& path, const std::string& parseMode, const std::string& cp, bool ro, bool log) try
{
    auto fullPath = std::filesystem::canonical(path);

    if (auto wnd = GetEditorWnd(fullPath))
        return WndManager::getInstance().SetTopWnd(wnd);

    if (!std::filesystem::is_regular_file(fullPath))
        throw std::runtime_error{ "Not regular file: " + fullPath.filename().u8string() };

    auto editor = std::make_shared<EditorWnd>();
    editor->Show(true, -1);
    if (editor->SetFileName(fullPath, false, parseMode, cp))
    {
        editor->SetRO(ro);
        editor->SetLog(log);
        m_editors[editor.get()] = editor;

        m_recentFiles.erase(std::remove_if(m_recentFiles.begin(), m_recentFiles.end(),
            [&fullPath](const file_t& file) {return std::get<0>(file) == fullPath;}), m_recentFiles.end());
        if (m_recentFiles.size() >= c_maxRecentFiles)
            m_recentFiles.pop_front();

        m_recentFiles.emplace_back(fullPath.u8string(), parseMode, cp, ro, log);
        UpdateRecentFilesList();
    }

    return true;
}
catch (const std::exception& ex)
{
    LOG(ERROR) << __FUNC__ << "Error file loading: " << ex.what();
    MsgBox(MBoxKey::OK, "Open: " + path.filename().u8string(),
        { "Error open file." },
        { "Close" }
    );
    EditorApp::SetErrorLine("Error file loading");
    return false;
}
catch (...)
{
    LOG(ERROR) << __FUNC__ << "Error file loading: unknown exeption";
    EditorApp::SetErrorLine("Error file loading");
    return false;
}

bool EditorApp::LoadCfg()
{
    //configuration loading
    LOG(DEBUG) << __FUNC__;
    
    auto cfgPath = Directory::CfgPath(EDITOR_NAME) / EditorConfig::ConfigDir / EditorConfig::ConfigFile;
    if (std::filesystem::exists(cfgPath))
        _TRY(g_editorConfig.Load(cfgPath));

    KeyConfig keyConfig;
    _TRY(keyConfig.Load(Directory::CfgPath(EDITOR_NAME) / EditorConfig::ConfigDir / g_editorConfig.keyFile));

    auto parserPath = Directory::ProgrammPath(EDITOR_NAME) / ParserConfig::ConfigDir / ("*" + ParserConfig::Ext);
    DirectoryList parserDir;
    parserDir.SetMask(parserPath);
    parserDir.Scan();
    auto& list = parserDir.GetFileList();
    for(auto& file : list)
    {
        ParserConfig parser;
        _TRY(parser.Load(file));
    }

    return true;
}

bool EditorApp::SaveCfg(input_t code)
{ 
    //configuration saving
    LOG(DEBUG) << __FUNC__;

    if (code == K_EXIT)
    {
        auto cfgPath = Directory::CfgPath(EDITOR_NAME) / EditorConfig::ConfigDir / EditorConfig::ConfigFile;
        if (std::filesystem::exists(cfgPath))
            _TRY(g_editorConfig.Save(cfgPath));
    }

    if (!m_editors.empty())
    {
        _TRY(SaveSession(std::nullopt));
    }

    return true;
}

bool EditorApp::SaveSession(std::optional<const std::filesystem::path> path)
{
    SessionConfig sesConfig;
    for (auto& [w, wnd] : m_editors)
    {
        WndConfig config;
        if (!wnd->SaveCfg(config))
            continue;

        sesConfig.SaveConfig(config);

        //update recent files info
        auto entry = std::find_if(m_recentFiles.begin(), m_recentFiles.end(),
            [&config](const file_t& file) {return config.filePath == std::get<0>(file); });

        if (entry != m_recentFiles.end())
            *entry = { config.filePath, config.parser, config.cp, config.ro, config.log };
    }

    for (auto& [filePath, parse, cp, ro, log] : m_recentFiles)
    {
        FileConfig config{ filePath, parse, cp, ro, log };
        sesConfig.SaveConfig(config);
    }

    auto GetFile = [](Wnd* wnd) -> std::string {
        if (!wnd || wnd->GetWndType() != wnd_t::editor)
            return {};
        auto wedit = reinterpret_cast<EditorWnd*>(wnd);
        return wedit->GetFilePath().u8string();
    };

    ViewConfig viewConfig;
    viewConfig.sizex    = WndManager::getInstance().m_splitX;
    viewConfig.sizey    = WndManager::getInstance().m_splitY;
    viewConfig.type     = static_cast<size_t>(WndManager::getInstance().m_splitType);
    viewConfig.active   = WndManager::getInstance().m_activeView;
    viewConfig.file1    = GetFile(WndManager::getInstance().GetWnd(0, 0));
    viewConfig.file2    = GetFile(WndManager::getInstance().GetWnd(0, 1));
    sesConfig.SaveConfig(viewConfig);

    DialogsConfig dlgConfig;
    dlgConfig.filePath      = FileDialog::s_vars.path;
    dlgConfig.fileMaskList  = FileDialog::s_vars.maskList;
    dlgConfig.findList      = FindDialog::s_vars.findList;
    dlgConfig.replaceList   = FindDialog::s_vars.replaceList;
    sesConfig.SaveConfig(dlgConfig);

    if (path)
        sesConfig.Save(*path);
    else
        sesConfig.Save(Directory::UserCfgPath(EDITOR_NAME) / SessionConfig::File);

    return true;
}

bool EditorApp::LoadSession(std::optional<const std::filesystem::path> path)
{
    LOG(DEBUG) << __FUNC__;

    SessionConfig sesConfig;
    bool rc;
    if (path)
        rc = sesConfig.Load(*path);
    else
        rc = sesConfig.Load(Directory::UserCfgPath(EDITOR_NAME) / SessionConfig::File);

    if (!rc)
        return rc;

    auto&& wndConfig    = sesConfig.GetConfig<std::vector<WndConfig>>();
    auto&& vConfig      = sesConfig.GetConfig<ViewConfig>();
    auto&& dConfig      = sesConfig.GetConfig<DialogsConfig>();
    auto&& fileConfig   = sesConfig.GetConfig<std::vector<FileConfig>>();

    for (auto& wConfig : wndConfig)
    {
        OpenFile(utf8::utf8to16(wConfig.filePath), wConfig.parser, wConfig.cp, wConfig.ro, wConfig.log);
        auto wnd = GetEditorWnd(wConfig.filePath);
        if (wnd)
        {
            auto eWnd = dynamic_cast<EditorWnd*>(wnd);
            eWnd->LoadCfg(wConfig);
        }
    }

    WndManager::getInstance().m_splitX      = vConfig.sizex;
    WndManager::getInstance().m_splitY      = vConfig.sizey;
    auto splitType = static_cast<split_t>(vConfig.type);
    auto activeView = vConfig.active;

    if (!vConfig.file1.empty())
    {
        auto wnd = GetEditorWnd(vConfig.file1);
        if(nullptr != wnd)
            WndManager::getInstance().SetTopWnd(wnd, 0);
        else
        {
            activeView = 0;
            splitType = split_t::no_split;
        }
    }
    if (!vConfig.file2.empty())
    {
        auto wnd = GetEditorWnd(vConfig.file2);
        if (nullptr != wnd)
            WndManager::getInstance().SetTopWnd(wnd, 1);
        else
        {
            activeView = 0;
            splitType = split_t::no_split;
        }
    }

    WndManager::getInstance().m_splitType = static_cast<split_t>(splitType);
    WndManager::getInstance().m_activeView = activeView;

    WndManager::getInstance().CalcView();

    FileDialog::s_vars.path         = dConfig.filePath;
    FileDialog::s_vars.maskList     = dConfig.fileMaskList;
    FindDialog::s_vars.findList     = dConfig.findList;
    FindDialog::s_vars.replaceList  = dConfig.replaceList;
    
    m_recentFiles.clear();
    for (auto& file : fileConfig)
        m_recentFiles.emplace_back(file.filePath, file.parser, file.cp, file.ro, file.log);
    UpdateRecentFilesList();

    return true;
}

} //namespace _Editor
