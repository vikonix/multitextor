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

#include "EditorCmd.h"
#include "EditorWnd.h"


/////////////////////////////////////////////////////////////////////////////
//first array is key array
//second array is cmd array
CmdMap g_defaultAppKeyMap
{
    {K_F1},                     {K_APP_HELP},

    {K_F2},                     {K_APP_SAVE_ALL},
    {'S' | K_ALT},              {K_APP_SAVE_ALL},

    {K_F3},                     {K_APP_DLG_OPEN},

    {K_F5 | K_SHIFT},           {K_APP_WND_COPY},
    {K_F6 | K_SHIFT},           {K_APP_WND_MOVE},

    {'F' | K_ALT},              {K_APP_FINDFILE},
    {K_ESC, K_F7},              {K_APP_FINDFILE},
    {K_ESC, 'f'},               {K_APP_FINDFILE},
    {K_ESC, 'F'},               {K_APP_FINDFILE},
    {'H' | K_ALT},              {K_APP_REPLACEFILE},
    {K_ESC, 'h'},               {K_APP_REPLACEFILE},
    {K_ESC, 'H'},               {K_APP_REPLACEFILE},

    {'L' | K_CTRL},             {K_APP_FOUNDFILE},
    {'L' | K_ALT},              {K_APP_FOUNDFILE},

    {'W' | K_CTRL},             {K_APP_WND_LIST},

    {K_F9},                     {K_MENU},
    {K_ESC, K_F1},              {K_MENU},

    {K_F10},                    {K_EXIT},
    {'X' | K_ALT},              {K_EXIT},
    {K_ESC, 'x'},               {K_EXIT},
    {K_ESC, 'X'},               {K_EXIT},

    {K_ESC, '0'},               {K_INSERT},

    {'/' | K_ALT},              {K_APP_VIEW_SPLIT},
    {'+' | K_ALT},              {K_APP_VIEW_MODE},
    {'=' | K_ALT},              {K_APP_VIEW_MODE},
    {'-' | K_ALT},              {K_APP_VIEW_SIZE},
    {K_ESC, '/'},               {K_APP_VIEW_SPLIT},
    {K_ESC, '='},               {K_APP_VIEW_MODE},
    {K_ESC, '+'},               {K_APP_VIEW_MODE},
    {K_ESC, '-'},               {K_APP_VIEW_SIZE},
    {K_PAGEUP | K_ALT},         {K_APP_VIEW_SET},
    {K_PAGEDN | K_ALT},         {K_APP_VIEW_SET},
    {K_ESC, K_PAGEUP},          {K_APP_VIEW_SET},
    {K_ESC, K_PAGEDN},          {K_APP_VIEW_SET},
    {K_ESC, '9'},               {K_APP_VIEW_SET},
    {K_ESC, '3'},               {K_APP_VIEW_SET},

    {'D' | K_CTRL},             {K_APP_DIFF},
    {'\\' | K_CTRL},            {K_APP_BOOKMARK_LIST},
    {'\\' | K_ALT},             {K_APP_BOOKMARK_LIST},

    {'0' | K_ALT},              {K_APP_BOOKMARK_0},
    {'1' | K_ALT},              {K_APP_BOOKMARK_1},
    {'2' | K_ALT},              {K_APP_BOOKMARK_2},
    {'3' | K_ALT},              {K_APP_BOOKMARK_3},
    {'4' | K_ALT},              {K_APP_BOOKMARK_4},
    {'5' | K_ALT},              {K_APP_BOOKMARK_5},
    {'6' | K_ALT},              {K_APP_BOOKMARK_6},
    {'7' | K_ALT},              {K_APP_BOOKMARK_7},
    {'8' | K_ALT},              {K_APP_BOOKMARK_8},
    {'9' | K_ALT},              {K_APP_BOOKMARK_9},

    {'K' | K_CTRL},             {K_APP_RECORD_MACRO},
    {'K' | K_ALT},              {K_APP_PLAY_MACRO},
    {'Q' | K_CTRL},             {K_APP_KEYGEN},

    //for test
    {K_F12},                    {K_REFRESH}
};



CmdMap g_defaultEditKeyMap
{
    {K_LEFT},                   {K_ED(E_MOVE_LEFT)},
    {K_RIGHT},                  {K_ED(E_MOVE_RIGHT)},
    {K_UP},                     {K_ED(E_MOVE_UP)},
    {K_DOWN},                   {K_ED(E_MOVE_DOWN)},
    {K_MOUSEWUP | K_MOUSEW},    {K_ED(E_MOVE_UP) + 3},
    {K_MOUSEWDN | K_MOUSEW},    {K_ED(E_MOVE_DOWN) + 3},
    {K_PAGEUP},                 {K_ED(E_MOVE_PAGE_UP)},
    {K_PAGEDN},                 {K_ED(E_MOVE_PAGE_DOWN)},
    {K_HOME},                   {K_ED(E_MOVE_STR_BEGIN)},
    {K_END},                    {K_ED(E_MOVE_STR_END)},
    {K_HOME | K_CTRL},          {K_ED(E_MOVE_FILE_BEGIN)},
    {K_END | K_CTRL},           {K_ED(E_MOVE_FILE_END)},
    {K_HOME | K_ALT},           {K_ED(E_MOVE_FILE_BEGIN)},
    {K_END | K_ALT},            {K_ED(E_MOVE_FILE_END)},
    {K_ESC, K_HOME},            {K_ED(E_MOVE_FILE_BEGIN)},
    {K_ESC, K_END},             {K_ED(E_MOVE_FILE_END)},
    {K_ESC, '7'},               {K_ED(E_MOVE_FILE_BEGIN)},
    {K_ESC, '1'},               {K_ED(E_MOVE_FILE_END)},
    {K_LEFT | K_CTRL},          {K_ED(E_MOVE_WORD_LEFT)},
    {K_RIGHT | K_CTRL},         {K_ED(E_MOVE_WORD_RIGHT)},
    {',' | K_ALT},              {K_ED(E_MOVE_WORD_LEFT)},
    {'.' | K_ALT},              {K_ED(E_MOVE_WORD_RIGHT)},
    {'<' | K_ALT},              {K_ED(E_MOVE_WORD_LEFT)},
    {'>' | K_ALT},              {K_ED(E_MOVE_WORD_RIGHT)},
    {K_ESC, ','},               {K_ED(E_MOVE_WORD_LEFT)},
    {K_ESC, '.'},               {K_ED(E_MOVE_WORD_RIGHT)},
    {']' | K_CTRL},             {K_ED(E_MOVE_LEX_MATCH)},
    {'[' | K_CTRL},             {K_ED(E_MOVE_LEX_MATCH)},
    {'T' | K_CTRL},             {K_ED(E_MOVE_CENTER)},

    {K_UP | K_ALT},             {K_ED(E_MOVE_UP) + 1},
    {K_DOWN | K_ALT},           {K_ED(E_MOVE_DOWN) + 1},
    {K_LEFT | K_ALT},           {K_ED(E_MOVE_SCROLL_LEFT)},
    {K_RIGHT | K_ALT},          {K_ED(E_MOVE_SCROLL_RIGHT)},

    {K_DELETE},                 {K_ED(E_EDIT_DEL_C)},
    {K_ESC, K_BS},              {K_ED(E_EDIT_DEL_C)},
    {'Y' | K_CTRL},             {K_ED(E_EDIT_DEL_STR)},
    {K_ENTER},                  {K_ED(E_EDIT_ENTER)},
    {K_TAB},                    {K_ED(E_EDIT_TAB)},
    {K_BS},                     {K_ED(E_EDIT_BS)},

    {K_MOUSEKL | K_MOUSE2},     {K_ED(E_SELECT_WORD)},
    {K_MOUSEKL | K_MOUSE3},     {K_ED(E_SELECT_LINE)},
    {K_SPACE | K_CTRL},         {K_ED(E_SELECT_WORD)},
    {K_ESC, K_LEFT},            {K_ED(E_SELECT_BEGIN), K_ED(E_MOVE_LEFT)},
    {K_ESC, K_RIGHT},           {K_ED(E_SELECT_BEGIN), K_ED(E_MOVE_RIGHT)},
    {K_ESC, K_UP},              {K_ED(E_SELECT_BEGIN), K_ED(E_MOVE_UP)},
    {K_ESC, K_DOWN},            {K_ED(E_SELECT_BEGIN), K_ED(E_MOVE_DOWN)},
    {K_LEFT | K_SHIFT},         {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_LEFT)},
    {K_RIGHT | K_SHIFT},        {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_RIGHT)},
    {K_UP | K_SHIFT},           {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_UP)},
    {K_DOWN | K_SHIFT},         {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_DOWN)},
    {K_HOME | K_SHIFT},         {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_STR_BEGIN)},
    {K_END | K_SHIFT},          {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_STR_END)},
    {K_PAGEUP | K_SHIFT},       {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_PAGE_UP)},
    {K_PAGEDN | K_SHIFT},       {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_PAGE_DOWN)},
    {K_F4},                     {K_ED(E_SELECT_MODE)},
    {K_F4 | K_SHIFT},           {K_ED(E_SELECT_MODE) + 1},
    {K_F8},                     {K_ED(E_SELECT_UNSELECT)},
    {K_F4},                     {K_ED(E_SELECT_END)},
    {K_ESC, K_ESC},             {K_ED(E_SELECT_END)},
    {K_RELEASE | K_SHIFT},      {K_ED(E_SELECT_END) + 1},

    {K_F5},                     {K_ED(E_EDIT_BLOCK_COPY)},
    {K_F6},                     {K_ED(E_EDIT_BLOCK_MOVE)},
    {'Y' | K_ALT},              {K_ED(E_EDIT_BLOCK_DEL)},

    {'X' | K_CTRL},             {K_ED(E_EDIT_CB_CUT)},
    {'C' | K_CTRL},             {K_ED(E_EDIT_CB_COPY)},
    {'V' | K_CTRL},             {K_ED(E_EDIT_CB_PASTE)},
    {K_DELETE | K_SHIFT},       {K_ED(E_EDIT_CB_CUT)},
    {K_INSERT | K_CTRL},        {K_ED(E_EDIT_CB_COPY)},
    {K_INSERT | K_SHIFT},       {K_ED(E_EDIT_CB_PASTE)},
    {'I' | K_ALT},              {K_ED(E_EDIT_BLOCK_INDENT)},
    {'U' | K_ALT},              {K_ED(E_EDIT_BLOCK_UNDENT)},
    {K_ESC, 'i'},               {K_ED(E_EDIT_BLOCK_INDENT)},
    {K_ESC, 'I'},               {K_ED(E_EDIT_BLOCK_INDENT)},
    {K_ESC, 'u'},               {K_ED(E_EDIT_BLOCK_UNDENT)},
    {K_ESC, 'U'},               {K_ED(E_EDIT_BLOCK_UNDENT)},

    {'Z' | K_CTRL},             {K_ED(E_EDIT_UNDO)},
    {K_BS | K_ALT},             {K_ED(E_EDIT_UNDO)},
    {'Z' | K_ALT},              {K_ED(E_EDIT_REDO)},
    {K_ESC, 'z'},               {K_ED(E_EDIT_REDO)},
    {K_ESC, 'Z'},               {K_ED(E_EDIT_REDO)},

    {'G' | K_CTRL},             {K_ED(E_DLG_GOTO)},
    {'F' | K_CTRL},             {K_ED(E_DLG_FIND)},
    {K_F7},                     {K_ED(E_DLG_FIND)},
    {'H' | K_CTRL},             {K_ED(E_DLG_REPLACE)},

    {K_F7 | K_CTRL},            {K_ED(E_CTRL_FIND)},
    {K_F7 | K_SHIFT},           {K_ED(E_CTRL_REPEAT)},
    {'P' | K_CTRL},             {K_ED(E_CTRL_FINDUP)},
    {'N' | K_CTRL},             {K_ED(E_CTRL_FINDDN)},
    {'P' | K_ALT},              {K_ED(E_CTRL_FINDUPW)},
    {'N' | K_ALT},              {K_ED(E_CTRL_FINDDNW)},

    {'S' | K_CTRL},             {K_ED(E_CTRL_SAVE)},
    {K_ESC, K_F2},              {K_ED(E_CTRL_SAVE)},
    {K_F2 | K_SHIFT},           {K_ED(E_CTRL_SAVEAS)},
    {K_F3 | K_SHIFT},           {K_ED(E_CTRL_RELOAD)},
    {K_ESC, K_F3},              {K_ED(E_CTRL_RELOAD)},
    {K_F10 | K_SHIFT},          {K_ED(E_CTRL_CLOSE)},
    {K_ESC, K_F10},             {K_ED(E_CTRL_CLOSE)},

    {'M' | K_ALT},              {K_ED(E_POPUP_MENU)},
    {K_ESC, 'm'},               {K_ED(E_POPUP_MENU)},
    {K_ESC, 'M'},               {K_ED(E_POPUP_MENU)},

    { 'J' | K_CTRL },           { K_ED(E_CTRL_FUNC_LIST) },
//???    {'A' | K_CTRL},             {K_ED(E_CTRL_PROPERTIES)},
//???    {'M' | K_CTRL},             {K_ED(E_CTRL_CHANGE_CP)},

    //for testing
    { 'R' | K_ALT },            {K_ED(E_CTRL_REFRESH)}
};

std::unordered_map<EditorCmd, std::pair<EditorWnd::EditorFunc, EditorWnd::select_state>> EditorWnd::s_funcMap 
{
    {E_MOVE_LEFT,           {&EditorWnd::MoveLeft,               EditorWnd::select_state::begin}},
    {E_MOVE_RIGHT,          {&EditorWnd::MoveRight,              EditorWnd::select_state::begin}},
    {E_MOVE_UP,             {&EditorWnd::MoveUp,                 EditorWnd::select_state::begin}},
    {E_MOVE_DOWN,           {&EditorWnd::MoveDown,               EditorWnd::select_state::begin}},
    {E_MOVE_SCROLL_LEFT,    {&EditorWnd::MoveScrollLeft,         EditorWnd::select_state::begin}},
    {E_MOVE_SCROLL_RIGHT,   {&EditorWnd::MoveScrollRight,        EditorWnd::select_state::begin}},
    {E_MOVE_PAGE_UP,        {&EditorWnd::MovePageUp,             EditorWnd::select_state::begin}},
    {E_MOVE_PAGE_DOWN,      {&EditorWnd::MovePageDown,           EditorWnd::select_state::begin}},
    {E_MOVE_FILE_BEGIN,     {&EditorWnd::MoveFileBegin,          EditorWnd::select_state::begin}},
    {E_MOVE_FILE_END,       {&EditorWnd::MoveFileEnd,            EditorWnd::select_state::begin}},
    {E_MOVE_STR_BEGIN,      {&EditorWnd::MoveStrBegin,           EditorWnd::select_state::begin}},
    {E_MOVE_STR_END,        {&EditorWnd::MoveStrEnd,             EditorWnd::select_state::begin}},
    {E_MOVE_TAB_LEFT,       {&EditorWnd::MoveTabLeft,            EditorWnd::select_state::begin}},
    {E_MOVE_TAB_RIGHT,      {&EditorWnd::MoveTabRight,           EditorWnd::select_state::begin}},
    {E_MOVE_WORD_LEFT,      {&EditorWnd::MoveWordLeft,           EditorWnd::select_state::begin}},
    {E_MOVE_WORD_RIGHT,     {&EditorWnd::MoveWordRight,          EditorWnd::select_state::begin}},
    {E_MOVE_POS,            {&EditorWnd::MovePos,                EditorWnd::select_state::begin}},
    {E_MOVE_CENTER,         {&EditorWnd::MoveCenter,             EditorWnd::select_state::no}},

    {E_SELECT_WORD,         {&EditorWnd::SelectWord,             EditorWnd::select_state::no}},
    {E_SELECT_LINE,         {&EditorWnd::SelectLine,             EditorWnd::select_state::no}},
    {E_SELECT_ALL,          {&EditorWnd::SelectAll,              EditorWnd::select_state::no}},
    {E_SELECT_BEGIN,        {&EditorWnd::SelectBegin,            EditorWnd::select_state::no}},
    {E_SELECT_END,          {&EditorWnd::SelectEnd,              EditorWnd::select_state::no}},
    {E_SELECT_UNSELECT,     {&EditorWnd::SelectUnselect,         EditorWnd::select_state::no}},
    {E_SELECT_MODE,         {&EditorWnd::SelectMode,             EditorWnd::select_state::no}},

    {E_EDIT_C,              {&EditorWnd::EditC,                  EditorWnd::select_state::end}},
    {E_EDIT_DEL_C,          {&EditorWnd::EditDelC,               EditorWnd::select_state::end}},
    {E_EDIT_BS,             {&EditorWnd::EditBS,                 EditorWnd::select_state::end}},
    {E_EDIT_TAB,            {&EditorWnd::EditTab,                EditorWnd::select_state::end}},
    {E_EDIT_ENTER,          {&EditorWnd::EditEnter,              EditorWnd::select_state::end}},
    {E_EDIT_DEL_STR,        {&EditorWnd::EditDelStr,             EditorWnd::select_state::end}},
    {E_EDIT_DEL_BEGIN,      {&EditorWnd::EditDelBegin,           EditorWnd::select_state::end}},
    {E_EDIT_DEL_END,        {&EditorWnd::EditDelEnd,             EditorWnd::select_state::end}},

    {E_EDIT_BLOCK_COPY,     {&EditorWnd::EditBlockCopy,          EditorWnd::select_state::end}},
    {E_EDIT_BLOCK_MOVE,     {&EditorWnd::EditBlockMove,          EditorWnd::select_state::end}},
    {E_EDIT_BLOCK_DEL,      {&EditorWnd::EditBlockDel,           EditorWnd::select_state::end}},
    {E_EDIT_BLOCK_INDENT,   {&EditorWnd::EditBlockIndent,        EditorWnd::select_state::end}},
    {E_EDIT_BLOCK_UNDENT,   {&EditorWnd::EditBlockUndent,        EditorWnd::select_state::end}},
    {E_EDIT_CB_COPY,        {&EditorWnd::EditCopyToClipboard,    EditorWnd::select_state::end}},
    {E_EDIT_CB_CUT,         {&EditorWnd::EditCutToClipboard,     EditorWnd::select_state::end}},
    {E_EDIT_CB_PASTE,       {&EditorWnd::EditPasteFromClipboard, EditorWnd::select_state::end}},
    {E_EDIT_UNDO,           {&EditorWnd::EditUndo,               EditorWnd::select_state::end}},
    {E_EDIT_REDO,           {&EditorWnd::EditRedo,               EditorWnd::select_state::end}},

    {E_CTRL_FIND,           {&EditorWnd::CtrlFind,               EditorWnd::select_state::begin}},
    {E_CTRL_FINDUP,         {&EditorWnd::CtrlFindUp,             EditorWnd::select_state::begin}},
    {E_CTRL_FINDDN,         {&EditorWnd::CtrlFindDown,           EditorWnd::select_state::begin}},
    {E_CTRL_FINDUPW,        {&EditorWnd::FindUpWord,             EditorWnd::select_state::begin}},
    {E_CTRL_FINDDNW,        {&EditorWnd::FindDownWord,           EditorWnd::select_state::begin}},
    {E_CTRL_REPLACE,        {&EditorWnd::Replace,                EditorWnd::select_state::end}},
    {E_CTRL_REPEAT,         {&EditorWnd::Repeat,                 EditorWnd::select_state::end}},

    {E_DLG_GOTO,            {&EditorWnd::DlgGoto,                EditorWnd::select_state::end}},
    {E_DLG_FIND,            {&EditorWnd::DlgFind,                EditorWnd::select_state::end}},
    {E_DLG_REPLACE,         {&EditorWnd::DlgReplace,             EditorWnd::select_state::end}},

    {E_CTRL_REFRESH,        {&EditorWnd::CtrlRefresh,            EditorWnd::select_state::end}},
    {E_CTRL_RELOAD,         {&EditorWnd::Reload,                 EditorWnd::select_state::end}},
    {E_CTRL_SAVE,           {&EditorWnd::Save,                   EditorWnd::select_state::end}},
    {E_CTRL_SAVEAS,         {&EditorWnd::SaveAs,                 EditorWnd::select_state::end}},
    {E_CTRL_CLOSE,          {&EditorWnd::Close,                  EditorWnd::select_state::end}},

    {E_MOVE_LEX_MATCH,      {&EditorWnd::MoveLexMatch,           EditorWnd::select_state::begin}},
    {E_CTRL_FUNC_LIST,      {&EditorWnd::CtrlFuncList,           EditorWnd::select_state::end}},
    {E_CTRL_PROPERTIES,     {&EditorWnd::CtrlProperties,         EditorWnd::select_state::end}},
    {E_CTRL_CHANGE_CP,      {&EditorWnd::CtrlChangeCP,           EditorWnd::select_state::no}},
    {E_POPUP_MENU,          {&EditorWnd::TrackPopupMenu,         EditorWnd::select_state::end}}
};

std::unordered_map<input_t, std::string> g_CmdNames
{
    //Keys
    {K_SHIFT,                       "Shift"},
    {K_CTRL,                        "Ctrl"},
    {K_ALT,                         "Alt"},

    {K_F1,                          "F1"},
    {K_F2,                          "F2"},
    {K_F3,                          "F3"},
    {K_F4,                          "F4"},
    {K_F5,                          "F5"},
    {K_F6,                          "F6"},
    {K_F7,                          "F7"},
    {K_F8,                          "F8"},
    {K_F9,                          "F9"},
    {K_F10,                         "F10"},
    {K_F11,                         "F11"},
    {K_F12,                         "F12"},

    {K_ESC,                         "Esc"},
    {K_ENTER,                       "Enter"},
    {K_SPACE,                       "Space"},
    {K_BS,                          "BackSpace"},
    {K_TAB,                         "Tab"},

    {K_INSERT,                      "Insert"},
    {K_DELETE,                      "Delete"},
    {K_HOME,                        "Home"},
    {K_END,                         "End"},
    {K_PAGEUP,                      "PageUp"},
    {K_PAGEDN,                      "PageDn"},

    {K_UP,                          "Up"},
    {K_DOWN,                        "Down"},
    {K_LEFT,                        "Left"},
    {K_RIGHT,                       "Right"},

    {K_MOUSEWUP | K_MOUSEW,         "MouseScrollUp"},
    {K_MOUSEWDN | K_MOUSEW,         "MouseScrollDown"},
    {K_MOUSEKL | K_MOUSE2,          "MouseLeft2"},
    {K_MOUSEKL | K_MOUSE3,          "MouseLeft3"},

    {K_RELEASE | K_SHIFT,           "ShiftRelease"},

    //App cmd
    {K_EXIT,                        "APP_EXIT"},
    {K_MENU,                        "APP_MENU"},
    {K_REFRESH,                     "APP_REFRESH"},
    {K_INSERT,                      "APP_INSERT_MODE"},

    {K_APP_ABOUT,                   "APP_ABOUT"},
    {K_APP_HELP,                    "APP_HELP"},

    {K_APP_NEW,                     "APP_NEW"},
    {K_APP_SAVE_ALL,                "APP_SAVE_ALL"},
    {K_APP_DLG_OPEN,                "APP_DLG_OPEN"},

    {K_APP_FINDFILE,                "APP_DLG_FINDFILE"},
    {K_APP_REPLACEFILE,             "APP_DLG_REPLACEFILE"},
    {K_APP_FOUNDFILE,               "APP_DLG_FOUNDFILE"},
    {K_APP_WND_COPY,                "APP_DLG_WND_COPY"},
    {K_APP_WND_MOVE,                "APP_DLG_WND_MOVE"},

    {K_APP_WND_CLOSEALL,            "APP_WND_CLOSEALL"},
    {K_APP_WND_LIST,                "APP_DLG_WND_LIST"},

    {K_APP_VIEW_SPLIT,              "APP_VIEW_SPLIT_MERGE"},
    {K_APP_VIEW_MODE,               "APP_VIEW_SPLIT_VH"},
    {K_APP_VIEW_SET,                "APP_VIEW_SELECT"},
    {K_APP_VIEW_SIZE,               "APP_VIEW_SIZE"},

    {K_APP_DIFF,                    "APP_DLG_DIFF"},
    {K_APP_BOOKMARK_LIST,           "APP_DLG_BOOKMARK"},
    {K_APP_KEYGEN,                  "APP_DLG_KEYGEN"},
    {K_APP_NEW_SESSION,             "APP_DLG_NEW_SESSION"},
    {K_APP_OPEN_SESSION,            "APP_DLG_OPEN_SESSION"},

    {K_APP_BOOKMARK_0,              "APP_BOOKMARK_0"},
    {K_APP_BOOKMARK_1,              "APP_BOOKMARK_1"},
    {K_APP_BOOKMARK_2,              "APP_BOOKMARK_2"},
    {K_APP_BOOKMARK_3,              "APP_BOOKMARK_3"},
    {K_APP_BOOKMARK_4,              "APP_BOOKMARK_4"},
    {K_APP_BOOKMARK_5,              "APP_BOOKMARK_5"},
    {K_APP_BOOKMARK_6,              "APP_BOOKMARK_6"},
    {K_APP_BOOKMARK_7,              "APP_BOOKMARK_7"},
    {K_APP_BOOKMARK_8,              "APP_BOOKMARK_8"},
    {K_APP_BOOKMARK_9,              "APP_BOOKMARK_9"},

    {K_APP_RECORD_MACRO,            "APP_RECORD_MACRO"},
    {K_APP_PLAY_MACRO,              "APP_PLAY_MACRO"},

    {K_APP_COLOR,                   "APP_COLOR"},
    {K_APP_SETTINGS,                "APP_SETTINGS"},

    //Editor wnd
    { K_ED(E_MOVE_LEFT),            "EDIT_MOVE_LEFT"},
    { K_ED(E_MOVE_RIGHT),           "EDIT_MOVE_RIGHT"},
    { K_ED(E_MOVE_SCROLL_LEFT),     "EDIT_MOVE_SCROLL_LEFT"},
    { K_ED(E_MOVE_SCROLL_RIGHT),    "EDIT_MOVE_SCROLL_RIGHT"},
    { K_ED(E_MOVE_UP),              "EDIT_MOVE_UP"},
    { K_ED(E_MOVE_DOWN),            "EDIT_MOVE_DOWN"},
    { K_ED(E_MOVE_PAGE_UP),         "EDIT_MOVE_PAGE_UP"},
    { K_ED(E_MOVE_PAGE_DOWN),       "EDIT_MOVE_PAGE_DOWN"},
    { K_ED(E_MOVE_FILE_BEGIN),      "EDIT_MOVE_FILE_BEGIN"},
    { K_ED(E_MOVE_FILE_END),        "EDIT_MOVE_FILE_END"},
    { K_ED(E_MOVE_STR_BEGIN),       "EDIT_MOVE_STR_BEGIN"},
    { K_ED(E_MOVE_STR_END),         "EDIT_MOVE_STR_END"},
    { K_ED(E_MOVE_TAB_LEFT),        "EDIT_MOVE_TAB_LEFT"},
    { K_ED(E_MOVE_TAB_RIGHT),       "EDIT_MOVE_TAB_RIGHT"},
    { K_ED(E_MOVE_WORD_LEFT),       "EDIT_MOVE_WORD_LEFT"},
    { K_ED(E_MOVE_WORD_RIGHT),      "EDIT_MOVE_WORD_RIGHT"},
    { K_ED(E_MOVE_POS),             "EDIT_MOVE_POS"},
    { K_ED(E_MOVE_CENTER),          "EDIT_MOVE_CENTER"},

    { K_ED(E_SELECT_WORD),          "EDIT_SELECT_WORD"},
    { K_ED(E_SELECT_LINE),          "EDIT_SELECT_LINE"},
    { K_ED(E_SELECT_ALL),           "EDIT_SELECT_ALL"},
    { K_ED(E_SELECT_BEGIN),         "EDIT_SELECT_STREAM_BEGIN"},
    { K_ED(E_SELECT_END),           "EDIT_SELECT_END"},
    { K_ED(E_SELECT_UNSELECT),      "EDIT_SELECT_UNSELECT"},
    { K_ED(E_SELECT_MODE),          "EDIT_SELECT_LINES_BEGIN"},

    { K_ED(E_EDIT_DEL_C),           "EDIT_DEL_CHAR"},
    { K_ED(E_EDIT_BS),              "EDIT_BS"},
    { K_ED(E_EDIT_TAB),             "EDIT_TAB"},
    { K_ED(E_EDIT_ENTER),           "EDIT_ENTER"},
    { K_ED(E_EDIT_DEL_STR),         "EDIT_DEL_STR"},
    { K_ED(E_EDIT_DEL_BEGIN),       "EDIT_DEL_BEGIN"},
    { K_ED(E_EDIT_DEL_END),         "EDIT_DEL_END"},
    { K_ED(E_EDIT_BLOCK_CLEAR),     "EDIT_BLOCK_CLEAR"},
    { K_ED(E_EDIT_BLOCK_COPY),      "EDIT_BLOCK_COPY"},
    { K_ED(E_EDIT_BLOCK_MOVE),      "EDIT_BLOCK_MOVE"},
    { K_ED(E_EDIT_BLOCK_DEL),       "EDIT_BLOCK_DEL"},
    { K_ED(E_EDIT_BLOCK_INDENT),    "EDIT_BLOCK_INDENT"},
    { K_ED(E_EDIT_BLOCK_UNDENT),    "EDIT_BLOCK_UNDENT"},
    { K_ED(E_EDIT_CB_COPY),         "EDIT_CB_COPY"},
    { K_ED(E_EDIT_CB_CUT),          "EDIT_CB_CUT"},
    { K_ED(E_EDIT_CB_PASTE),        "EDIT_CB_PASTE"},
    { K_ED(E_EDIT_UNDO),            "EDIT_UNDO"},
    { K_ED(E_EDIT_REDO),            "EDIT_REDO"},

    { K_ED(E_CTRL_FIND),            "EDIT_CTRL_FIND"},
    { K_ED(E_CTRL_FINDUP),          "EDIT_CTRL_FINDUP"},
    { K_ED(E_CTRL_FINDDN),          "EDIT_CTRL_FINDDN"},
    { K_ED(E_CTRL_FINDUPW),         "EDIT_CTRL_FINDUPWORD"},
    { K_ED(E_CTRL_FINDDNW),         "EDIT_CTRL_FINDDNWORD"},
    { K_ED(E_CTRL_REPLACE),         "EDIT_CTRL_REPLACE"},
    { K_ED(E_CTRL_REPEAT),          "EDIT_CTRL_REPEAT"},
    { K_ED(E_DLG_GOTO),             "EDIT_DLG_GOTO"},
    { K_ED(E_DLG_FIND),             "EDIT_DLG_FIND"},
    { K_ED(E_DLG_REPLACE),          "EDIT_DLG_REPLACE"},

    { K_ED(E_CTRL_REFRESH),         "EDIT_CTRL_REFRESH"},
    { K_ED(E_CTRL_RELOAD),          "EDIT_CTRL_RELOAD"},
    { K_ED(E_CTRL_SAVE),            "EDIT_CTRL_SAVE"},
    { K_ED(E_CTRL_SAVEAS),          "EDIT_CTRL_SAVEAS"},
    { K_ED(E_CTRL_CLOSE),           "EDIT_CTRL_CLOSE"},

    { K_ED(E_MOVE_LEX_MATCH),       "EDIT_MOVE_BRACKET_MATCH"},
    { K_ED(E_CTRL_FUNC_LIST),       "EDIT_DLG_FUNC_LIST"},
    { K_ED(E_CTRL_PROPERTIES),      "EDIT_DLG_PROPERTIES"},
    { K_ED(E_CTRL_CHANGE_CP),       "EDIT_CTRL_CHANGE_CP"},
    { K_ED(E_POPUP_MENU),           "EDIT_POPUP_MENU"},

    //Editor additional
    { K_ED(E_MOVE_UP) + 1,          "EDIT_MOVE_SCROLL_UP"},
    { K_ED(E_MOVE_DOWN) + 1,        "EDIT_MOVE_SCROLL_DOWN"},
    { K_ED(E_SELECT_BEGIN) + 1,     "EDIT_SELECT_SHIFT_BEGIN"},
    { K_ED(E_SELECT_END) + 1,       "EDIT_SELECT_SHIFT_END"},
    { K_ED(E_SELECT_MODE) + 1,      "EDIT_SELECT_COLUMN_BEGIN"},
    { K_ED(E_CTRL_SAVE) + 1,        "EDIT_CTRL_OVERWRITE"},
    { K_ED(E_MOVE_UP) + 3,          "EDIT_MOVE_SCROLL3_UP"},
    { K_ED(E_MOVE_DOWN) + 3,        "EDIT_MOVE_SCROLL3_DOWN"}
};
