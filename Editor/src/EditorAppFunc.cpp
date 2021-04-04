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
#include "Dialogs/StdDialogs.h"
#include "Dialogs/EditorDialogs.h"
#include "utfcpp/utf8.h"


std::unordered_map<AppCmd, EditorApp::AppFunc> EditorApp::s_funcMap
{
    {K_APP_ABOUT,           &EditorApp::AboutProc},
    {K_APP_HELP,            &EditorApp::HelpProc},
    {K_APP_HELP_KEYMAP,     &EditorApp::HelpKeymapProc},

    {K_APP_NEW,             &EditorApp::FileNewProc},
    {K_APP_SAVE_ALL,        &EditorApp::FileSaveAllProc},
    {K_APP_DLG_OPEN,        &EditorApp::FileOpenProc},
    {K_APP_DLG_LOAD,        &EditorApp::FileLoadProc},
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


bool    EditorApp::FileOpenProc(input_t cmd)
{
    FileDialog dlg{ FileDlgMode::Open };
    auto rc = dlg.Activate();
    if (rc == ID_OK)
    {
        try
        {
            std::filesystem::path path{ utf8::utf8to16(dlg.s_vars.path) };
            path /= utf8::utf8to16(dlg.s_vars.file);

            auto editor = std::make_shared<EditorWnd>();
            editor->Show(true, -1);
            if(editor->SetFileName(path, false, dlg.s_vars.typeName, dlg.s_vars.cpName))
                m_editors[editor.get()] = editor;
        }
        catch (...)
        {
            _assert(0);
            LOG(ERROR) << "Error file loading";
            EditorApp::SetErrorLine("Error file loading");
        }
    }

    return true;
}

bool    EditorApp::WndListProc([[maybe_unused]] input_t cmd)
{
    WindowListDialog Dlg;
    [[maybe_unused]] auto code = Dlg.Activate();
    return true;
}

bool    EditorApp::WndCopyProc([[maybe_unused]] input_t cmd)
{
    WindowListDialog Dlg(WindowsDlgMode::CopyFrom);
    [[maybe_unused]] auto code = Dlg.Activate();
    return true;
}

bool    EditorApp::WndMoveProc([[maybe_unused]] input_t cmd)
{
    WindowListDialog Dlg(WindowsDlgMode::MoveFrom);
    [[maybe_unused]] auto code = Dlg.Activate();
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

bool    EditorApp::RecordMacroProc(input_t cmd)
{
    auto rc = MsgBox(
        "Macro Recording",
        "Warning!",
        !IsRecordMacro() ?
            "Do you want to start macro recording?" :
            "Do you want to stop macro recording?",
        MBOX_OK_CANCEL
    );

    if (rc == ID_OK)
        RecordMacro();

    return true;
}

bool    EditorApp::PlayMacroProc(input_t cmd)
{
    PlayMacro();
    return true;
}

bool    EditorApp::FileLoadProc(input_t cmd)
{
    return true;
}

bool    EditorApp::WndCloseAllProc(input_t cmd)
{
    return true;
}

bool    EditorApp::FindInFilesProc(input_t cmd)
{
    return true;
}

bool    EditorApp::ReplaceInFilesProc(input_t cmd)
{
    return true;
}

bool    EditorApp::FoundFilesProc(input_t cmd)
{
    return true;
}

bool    EditorApp::AboutProc(input_t cmd)
{
    return true;
}

bool    EditorApp::HelpProc(input_t cmd)
{
    return true;
}

bool    EditorApp::HelpKeymapProc(input_t cmd)
{
    return true;
}

bool    EditorApp::FileNewProc(input_t cmd)
{
    return true;
}

bool    EditorApp::FileSaveAllProc(input_t cmd)
{
    return true;
}

bool    EditorApp::DiffProc(input_t cmd)
{
    return true;
}

bool    EditorApp::BookmarkListProc(input_t cmd)
{
    return true;
}

bool    EditorApp::KeygenProc(input_t cmd)
{
    return true;
}

bool    EditorApp::NewSessionProc(input_t cmd)
{
    return true;
}

bool    EditorApp::OpenSessionProc(input_t cmd)
{
    return true;
}

bool    EditorApp::ColorDlgProc(input_t cmd)
{
    return true;
}

bool    EditorApp::SettingsDlgProc(input_t cmd)
{
    return true;
}

bool    EditorApp::SelectBookmarkProc(input_t cmd)
{
    return true;
}

bool    EditorApp::SelectRecentFileProc(input_t cmd)
{
    return true;
}

bool    EditorApp::SelectRecentSessionProc(input_t cmd)
{
    return true;
}
