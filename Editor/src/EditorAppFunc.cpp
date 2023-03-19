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
#include "EditorApp.h"
#include "Dialogs/EditorDialogs.h"
#include "Diff.h"
#include "utfcpp/utf8.h"
#include "Version.h"

namespace _Editor
{

std::unordered_map<AppCmd, EditorApp::AppFunc> EditorApp::s_cmdMap
{
    {K_APP_ABOUT,           &EditorApp::AboutCmd},
    {K_APP_HELP,            &EditorApp::HelpCmd},
    {K_APP_HELP_KEYMAP,     &EditorApp::HelpKeymapCmd},

    {K_APP_NEW,             &EditorApp::FileNewCmd},
    {K_APP_SAVE_ALL,        &EditorApp::FileSaveAllCmd},
    {K_APP_DLG_OPEN,        &EditorApp::FileOpenCmd},
    {K_APP_WND_CLOSEALL,    &EditorApp::WndCloseAllCmd},
    {K_APP_WND_LIST,        &EditorApp::WndListCmd},


    {K_APP_FINDFILE,        &EditorApp::FindInFilesCmd},
    {K_APP_REPLACEFILE,     &EditorApp::ReplaceInFilesCmd},
    {K_APP_FOUNDFILE,       &EditorApp::FoundFilesCmd},
    {K_APP_WND_COPY,        &EditorApp::WndCopyCmd},
    {K_APP_WND_MOVE,        &EditorApp::WndMoveCmd},

    {K_APP_VIEW_SPLIT,      &EditorApp::ViewSplitCmd},
    {K_APP_VIEW_MODE,       &EditorApp::ViewModeCmd},
    {K_APP_VIEW_SET,        &EditorApp::ViewSelectCmd},
    {K_APP_VIEW_SIZE,       &EditorApp::ViewMoveCmd},

    {K_APP_DIFF,            &EditorApp::DiffCmd},
    {K_APP_BOOKMARK_LIST,   &EditorApp::BookmarkListCmd},
    {K_APP_KEYGEN,          &EditorApp::KeygenCmd},
    {K_APP_NEW_SESSION,     &EditorApp::NewSessionCmd},
    {K_APP_OPEN_SESSION,    &EditorApp::OpenSessionCmd},

    {K_APP_RECORD_MACRO,    &EditorApp::RecordMacroCmd},
    {K_APP_PLAY_MACRO,      &EditorApp::PlayMacroCmd},

    {K_APP_COLOR,           &EditorApp::ColorDlgCmd},
    {K_APP_SETTINGS,        &EditorApp::SettingsDlgCmd},

    {K_APP_BOOKMARK,        &EditorApp::SelectBookmarkCmd},
    {K_APP_FILE_RECENT,     &EditorApp::SelectRecentFileCmd},
    {K_APP_SESSION_RECENT,  &EditorApp::SelectRecentSessionCmd}
};


bool    EditorApp::FileOpenCmd([[maybe_unused]] input_t cmd)
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

bool    EditorApp::FileNewCmd([[maybe_unused]] input_t cmd)
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

bool    EditorApp::WndCloseAllCmd([[maybe_unused]] input_t cmd)
{
    return CloseAllWindows();
}

bool    EditorApp::WndListCmd([[maybe_unused]] input_t cmd)
{
    WindowListDialog dlg;
    [[maybe_unused]] auto ret = dlg.Activate();
    return true;
}

bool    EditorApp::WndCopyCmd([[maybe_unused]] input_t cmd)
{
    WindowListDialog dlg(WindowsDlgMode::CopyFrom);
    [[maybe_unused]] auto ret = dlg.Activate();
    return true;
}

bool    EditorApp::WndMoveCmd([[maybe_unused]] input_t cmd)
{
    WindowListDialog dlg(WindowsDlgMode::MoveFrom);
    [[maybe_unused]] auto ret = dlg.Activate();
    return true;
}

bool    EditorApp::ViewSplitCmd([[maybe_unused]] input_t cmd)
{
    return WndManager::getInstance().ChangeViewMode();
}

bool    EditorApp::ViewModeCmd([[maybe_unused]] input_t cmd)
{
    return WndManager::getInstance().ChangeViewMode(1);
}

bool    EditorApp::ViewSelectCmd([[maybe_unused]] input_t cmd)
{
    return WndManager::getInstance().SetActiveView();
}

bool    EditorApp::ViewMoveCmd([[maybe_unused]] input_t cmd)
{
    return WndManager::getInstance().TrackView("Track view");
}

bool    EditorApp::RecordMacroCmd([[maybe_unused]] input_t cmd)
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

bool    EditorApp::PlayMacroCmd([[maybe_unused]] input_t cmd)
{
    PlayMacro();
    return true;
}

bool    EditorApp::AboutCmd([[maybe_unused]] input_t cmd)
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

bool    EditorApp::FindInFilesCmd([[maybe_unused]] input_t cmd)
{
    FindFileDialog dlg(false);
    auto ret = dlg.Activate();
    if (ret == ID_OK)
    {
        FileSaveAllCmd(0);
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

bool    EditorApp::ReplaceInFilesCmd([[maybe_unused]] input_t cmd)
{
    FindFileDialog dlg(true);
    auto ret = dlg.Activate();
    if (ret == ID_OK)
    {
        FileSaveAllCmd(0);
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

bool    EditorApp::FoundFilesCmd([[maybe_unused]] input_t cmd)
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

bool    EditorApp::HelpCmd(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::HelpKeymapCmd(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::FileSaveAllCmd([[maybe_unused]]input_t cmd)
{
    for (auto & [w, wnd] : m_editors)
    {
        wnd->Save(0);
    }
    return true;
}

bool    EditorApp::DiffCmd([[maybe_unused]] input_t cmd)
{
#if 0 //???
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
#else    
    DiffDialog mDlg(false);
    auto ret = mDlg.Activate();
    if (ret == ID_OK)
    {
        auto wnd1 = WndManager::getInstance().GetWnd(0, 0);
        auto wnd2 = WndManager::getInstance().GetWnd(0, 1);
        auto edWnd1 = dynamic_cast<EditorWnd*>(wnd1);
        auto edWnd2 = dynamic_cast<EditorWnd*>(wnd2);

        size_t first1{}, last1{STR_NOTDEFINED};
        size_t first2{}, last2{STR_NOTDEFINED};
        auto where = DiffDialog::s_vars.diffArea;
        if (where == 1)//from current position
        {
            first1 = edWnd1->GetCurStr();
            first2 = edWnd2->GetCurStr();
        }
        else if (where == 2)//restrict in selected area
        {
            edWnd1->GetSelectedLines(first1, last1);
            edWnd2->GetSelectedLines(first2, last2);
        }
        if (last1 == STR_NOTDEFINED)
        {
            last1 = edWnd1->GetEditor()->GetStrCount() - 1;
        }
        if (last2 == STR_NOTDEFINED)
        {
            last2 = edWnd2->GetEditor()->GetStrCount() - 1;
        }

        Diff diff{edWnd1->GetEditor(), edWnd2->GetEditor(), DiffDialog::s_vars.diffWithoutSpace, first1, last1, first2, last2};
        auto d = diff.Compare();
        return d;
    }
#endif
    return true;
}

bool    EditorApp::BookmarkListCmd(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::KeygenCmd(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::NewSessionCmd(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::OpenSessionCmd(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::ColorDlgCmd(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::SettingsDlgCmd(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::SelectBookmarkCmd(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

bool    EditorApp::SelectRecentFileCmd(input_t cmd)
{
    input_t n = cmd - K_APP_FILE_RECENT;
    if (m_recentFiles.size() <= n)
        return true;

    auto& [path, parse, cp, ro, log] = m_recentFiles[n];
    return OpenFile(utf8::utf8to16(path), parse, cp, ro, log);
}

bool    EditorApp::SelectRecentSessionCmd(input_t cmd)
{
    LOG(DEBUG) << __FUNC__ << " not implemented";
    SetErrorLine("Command not implemented");
    return true;
}

} //namespace _Editor
