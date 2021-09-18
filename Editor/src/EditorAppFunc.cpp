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
#include "utfcpp/utf8.h"
#include "Version.h"

namespace _Editor
{

std::unordered_map<AppCmd, EditorApp::AppFunc> EditorApp::s_funcMap
{
    {K_APP_ABOUT,           &EditorApp::AboutProc},
    {K_APP_HELP,            &EditorApp::HelpProc},
    {K_APP_HELP_KEYMAP,     &EditorApp::HelpKeymapProc},

    {K_APP_NEW,             &EditorApp::FileNewProc},
    {K_APP_SAVE_ALL,        &EditorApp::FileSaveAllProc},
    {K_APP_DLG_OPEN,        &EditorApp::FileOpenProc},
    {K_APP_WND_CLOSEALL,    &EditorApp::WndCloseAllProc},
    {K_APP_WND_LIST,        &EditorApp::WndListProc},


    {K_APP_FINDFILE,        &EditorApp::FindInFilesProc},
    {K_APP_REPLACEFILE,     &EditorApp::ReplaceInFilesProc},
    {K_APP_FOUNDFILE,       &EditorApp::FoundFilesProc},
    {K_APP_WND_COPY,        &EditorApp::WndCopyProc},
    {K_APP_WND_MOVE,        &EditorApp::WndMoveProc},

    {K_APP_VIEW_SPLIT,      &EditorApp::ViewSplitProc},
    {K_APP_VIEW_MODE,       &EditorApp::ViewModeProc},
    {K_APP_VIEW_SET,        &EditorApp::ViewSelectProc},
    {K_APP_VIEW_SIZE,       &EditorApp::ViewMoveProc},

    {K_APP_DIFF,            &EditorApp::DiffProc},
    {K_APP_BOOKMARK_LIST,   &EditorApp::BookmarkListProc},
    {K_APP_KEYGEN,          &EditorApp::KeygenProc},
    {K_APP_NEW_SESSION,     &EditorApp::NewSessionProc},
    {K_APP_OPEN_SESSION,    &EditorApp::OpenSessionProc},

    {K_APP_RECORD_MACRO,    &EditorApp::RecordMacroProc},
    {K_APP_PLAY_MACRO,      &EditorApp::PlayMacroProc},

    {K_APP_COLOR,           &EditorApp::ColorDlgProc},
    {K_APP_SETTINGS,        &EditorApp::SettingsDlgProc},

    {K_APP_BOOKMARK,        &EditorApp::SelectBookmarkProc},
    {K_APP_FILE_RECENT,     &EditorApp::SelectRecentFileProc},
    {K_APP_SESSION_RECENT,  &EditorApp::SelectRecentSessionProc}
};


bool    EditorApp::FileOpenProc([[maybe_unused]] input_t cmd)
{
    FileDialog dlg{ FileDlgMode::Open };
    auto ret = dlg.Activate();
    if (ret == ID_OK)
    {
        std::filesystem::path path{ utf8::utf8to16(dlg.s_vars.path) };
        path /= utf8::utf8to16(dlg.s_vars.file);
        return OpenFile(path, dlg.s_vars.typeName, dlg.s_vars.cpName, dlg.s_vars.ro, dlg.s_vars.log);
    }

    return true;
}

bool    EditorApp::FileNewProc([[maybe_unused]] input_t cmd)
{
    for (size_t n = 0; n < 1000; ++n)
    {
        std::filesystem::path path{ FileDialog::s_vars.path };
        path /= "untitled_" + std::to_string(n) + ".txt";

        if (!std::filesystem::exists(path) && GetEditorWnd(path) == nullptr)
        {
            auto editor = std::make_shared<EditorWnd>();
            editor->Show(true, -1);
            if (editor->SetFileName(path, true))
                m_editors[editor.get()] = editor;
            
            return true;
        }
    }
    return false;
}

bool    EditorApp::WndCloseAllProc([[maybe_unused]] input_t cmd)
{
    return CloseAllWindows();
}

bool    EditorApp::WndListProc([[maybe_unused]] input_t cmd)
{
    WindowListDialog dlg;
    [[maybe_unused]] auto ret = dlg.Activate();
    return true;
}

bool    EditorApp::WndCopyProc([[maybe_unused]] input_t cmd)
{
    WindowListDialog dlg(WindowsDlgMode::CopyFrom);
    [[maybe_unused]] auto ret = dlg.Activate();
    return true;
}

bool    EditorApp::WndMoveProc([[maybe_unused]] input_t cmd)
{
    WindowListDialog dlg(WindowsDlgMode::MoveFrom);
    [[maybe_unused]] auto ret = dlg.Activate();
    return true;
}

bool    EditorApp::ViewSplitProc([[maybe_unused]] input_t cmd)
{
    return WndManager::getInstance().ChangeViewMode();
}

bool    EditorApp::ViewModeProc([[maybe_unused]] input_t cmd)
{
    return WndManager::getInstance().ChangeViewMode(1);
}

bool    EditorApp::ViewSelectProc([[maybe_unused]] input_t cmd)
{
    return WndManager::getInstance().SetActiveView();
}

bool    EditorApp::ViewMoveProc([[maybe_unused]] input_t cmd)
{
    return WndManager::getInstance().TrackView("Track view");
}

bool    EditorApp::RecordMacroProc([[maybe_unused]] input_t cmd)
{
    auto ret = MsgBox(MBoxKey::OK_CANCEL, "Macro Recording",
        { "Warning!",
        !IsRecordMacro() ?
            "Do you want to start macro recording?" :
            "Do you want to stop macro recording?" }
    );

    if (ret == ID_OK)
        RecordMacro();

    return true;
}

bool    EditorApp::PlayMacroProc([[maybe_unused]] input_t cmd)
{
    PlayMacro();
    return true;
}

bool    EditorApp::AboutProc([[maybe_unused]] input_t cmd)
{
    MsgBox(MBoxKey::OK, "About",
        { EDITOR_NAME_C " Version: " EDITOR_VERSION,
        "Cross-platform text editor",
        "",
        "https://github.com/vikonix/multitextor",
        "Copyright (C) " COPYRIGHT_YEAR " " COPYRIGTH_OWNER}
    );
    return true;
}

bool    EditorApp::FindInFilesProc([[maybe_unused]] input_t cmd)
{
    FindFileDialog dlg(false);
    auto ret = dlg.Activate();
    if (ret == ID_OK)
    {
        FileSaveAllProc(0);
        SearchFileDialog sdlg;
        ret = sdlg.Activate();

        MatchedFileDialog mdlg;
        ret = mdlg.Activate();
        if (ret == ID_OK)
        {
            std::filesystem::path path = mdlg.s_path;
            auto [t, parser] = LexParser::GetFileType(path);
            auto rc = OpenFile(path, parser, FileDialog::s_vars.cpName);
            if (rc)
                PutCode(K_ED(E_CTRL_FINDDN));
        }
    }

    return true;
}

bool    EditorApp::ReplaceInFilesProc([[maybe_unused]] input_t cmd)
{
    FindFileDialog dlg(true);
    auto ret = dlg.Activate();
    if (ret == ID_OK)
    {
        FileSaveAllProc(0);
        SearchFileDialog sdlg;
        ret = sdlg.Activate();

        MatchedFileDialog mdlg;
        ret = mdlg.Activate();
        if (ret == ID_OK)
        {
            std::filesystem::path path = mdlg.s_path;
            auto [t, parser] = LexParser::GetFileType(path);
            auto rc = OpenFile(path, parser, FileDialog::s_vars.cpName);
            if (rc)
                PutCode(K_ED(E_CTRL_REPEAT));
        }
    }

    return true;
}

bool    EditorApp::FoundFilesProc([[maybe_unused]] input_t cmd)
{
    MatchedFileDialog mDlg;
    auto ret = mDlg.Activate();
    if (ret == ID_OK)
    {
        std::filesystem::path path = mDlg.s_path;
        auto [t, parser] = LexParser::GetFileType(path);
        ret = OpenFile(path, parser, FileDialog::s_vars.cpName);
        if (ret)
            PutCode(K_ED(E_CTRL_REPEAT));
    }

    return true;
}

bool    EditorApp::HelpProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::HelpKeymapProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::FileSaveAllProc([[maybe_unused]]input_t cmd)
{
    for (auto & [w, wnd] : m_editors)
    {
        wnd->Save(0);
    }
    return true;
}

bool    EditorApp::DiffProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::BookmarkListProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::KeygenProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::NewSessionProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::OpenSessionProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::ColorDlgProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::SettingsDlgProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::SelectBookmarkProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::SelectRecentFileProc(input_t cmd)
{
    input_t n = cmd - K_APP_FILE_RECENT;
    if (m_recentFiles.size() <= n)
        return true;

    auto& [path, parse, cp, ro, log] = m_recentFiles[n];
    return OpenFile(utf8::utf8to16(path), parse, cp, ro, log);
}

bool    EditorApp::SelectRecentSessionProc(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

} //namespace _Editor
