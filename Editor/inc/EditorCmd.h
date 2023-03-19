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
#pragma once

#include "Console/Types.h"
#include "Console/KeyCodes.h"

#include <unordered_map>
#include <string>

using namespace _Console;

namespace _Editor
{

enum AppCmd : input_t
{
    K_APP_ABOUT = K_APP,
    K_APP_HELP,
    K_APP_HELP_KEYMAP,

    K_APP_NEW,
    K_APP_SAVE_ALL,
    K_APP_DLG_OPEN,
    K_APP_WND_CLOSEALL,
    K_APP_WND_LIST,

    K_APP_FINDFILE,
    K_APP_REPLACEFILE,
    K_APP_FOUNDFILE,
    K_APP_WND_COPY,
    K_APP_WND_MOVE,

    K_APP_VIEW_SPLIT,
    K_APP_VIEW_MODE,
    K_APP_VIEW_SET,
    K_APP_VIEW_SIZE,

    K_APP_DIFF,
    K_APP_BOOKMARK_LIST,
    K_APP_KEYGEN,
    K_APP_NEW_SESSION,
    K_APP_OPEN_SESSION,

    K_APP_RECORD_MACRO,
    K_APP_PLAY_MACRO,

    K_APP_COLOR,
    K_APP_SETTINGS,

    K_APP_BOOKMARK,
    K_APP_BOOKMARK_0 = K_APP_BOOKMARK,
    K_APP_BOOKMARK_1,
    K_APP_BOOKMARK_2,
    K_APP_BOOKMARK_3,
    K_APP_BOOKMARK_4,
    K_APP_BOOKMARK_5,
    K_APP_BOOKMARK_6,
    K_APP_BOOKMARK_7,
    K_APP_BOOKMARK_8,
    K_APP_BOOKMARK_9,

    K_APP_FILE_RECENT = K_APP_BOOKMARK_9 + 100,
    K_APP_SESSION_RECENT = K_APP_FILE_RECENT + 100,
    K_APP_END = K_APP_SESSION_RECENT + 100
};

enum EditorCmd : input_t
{
    E_MOVE_LEFT,
    E_MOVE_RIGHT,
    E_MOVE_UP,
    E_MOVE_DOWN,
    E_MOVE_SCROLL_LEFT,
    E_MOVE_SCROLL_RIGHT,
    E_MOVE_PAGE_UP,
    E_MOVE_PAGE_DOWN,
    E_MOVE_FILE_BEGIN,
    E_MOVE_FILE_END,
    E_MOVE_STR_BEGIN,
    E_MOVE_STR_END,
    E_MOVE_TAB_LEFT,
    E_MOVE_TAB_RIGHT,
    E_MOVE_WORD_LEFT,
    E_MOVE_WORD_RIGHT,
    E_MOVE_POS,
    E_MOVE_CENTER,

    E_SELECT_WORD,
    E_SELECT_LINE,
    E_SELECT_ALL,
    E_SELECT_BEGIN,
    E_SELECT_END,
    E_SELECT_UNSELECT,
    E_SELECT_MODE,
    E_SELECT_ALL_FOUND,

    E_EDIT_C,
    E_EDIT_DEL_C,
    E_EDIT_BS,
    E_EDIT_TAB,
    E_EDIT_ENTER,
    E_EDIT_DEL_STR,
    E_EDIT_DEL_BEGIN,
    E_EDIT_DEL_END,
    E_EDIT_BLOCK_CLEAR,
    E_EDIT_BLOCK_COPY,
    E_EDIT_BLOCK_MOVE,
    E_EDIT_BLOCK_DEL,
    E_EDIT_BLOCK_INDENT,
    E_EDIT_BLOCK_UNINDENT,
    E_EDIT_CB_COPY,
    E_EDIT_CB_CUT,
    E_EDIT_CB_PASTE,
    E_EDIT_UNDO,
    E_EDIT_REDO,

    E_CTRL_FIND,
    E_CTRL_FINDUP,
    E_CTRL_FINDDN,
    E_CTRL_FINDUPW,
    E_CTRL_FINDDNW,
    E_CTRL_REPLACE,
    E_CTRL_REPEAT,
    E_DLG_GOTO,
    E_DLG_FIND,
    E_DLG_REPLACE,

    E_CTRL_REFRESH,
    E_CTRL_RELOAD,
    E_CTRL_SAVE,
    E_CTRL_SAVEAS,
    E_CTRL_CLOSE,

    E_MOVE_LEX_MATCH,
    E_CTRL_FUNC_LIST,
    E_CTRL_PROPERTIES,
    E_POPUP_MENU
};

#define EDITOR_CMD   K_USER
#define K_ED(n) (EDITOR_CMD + ((n) << 16))

extern CmdMap g_defaultEditKeyMap;
extern CmdMap g_defaultAppKeyMap;
extern CmdMap g_EditKeyMap;
extern CmdMap g_AppKeyMap;
extern std::unordered_map<input_t, std::string> g_CmdNames;

} //namespace _Editor
