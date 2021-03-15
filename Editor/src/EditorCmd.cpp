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
    {'T' | K_CTRL},             {K_ED(E_MOVE_CENTER)},

    {K_UP | K_ALT},             {K_ED(E_MOVE_UP) + 1},
    {K_DOWN | K_ALT},           {K_ED(E_MOVE_DOWN) + 1},
    {K_LEFT | K_ALT},           {K_ED(E_MOVE_SCROLL_LEFT)},
    {K_RIGHT | K_ALT},          {K_ED(E_MOVE_SCROLL_RIGHT)},

    {K_DELETE},                 {K_ED(E_EDIT_DEL_C)},
    {K_ESC, K_BS},              {K_ED(E_EDIT_DEL_C)},
    {'Y' | K_CTRL},             {K_ED(E_EDIT_DEL_STR)},
    {'B' | K_CTRL},             {K_ED(E_EDIT_DEL_BEGIN)},
    {'E' | K_CTRL},             {K_ED(E_EDIT_DEL_END)},

    {K_MOUSEKL | K_MOUSE2},     {K_ED(E_SELECT_WORD)},
    {K_MOUSEKL | K_MOUSE3},     {K_ED(E_SELECT_LINE)},
    {K_SPACE | K_CTRL},         {K_ED(E_SELECT_WORD)},
    {K_ESC, K_LEFT},            {K_ED(E_SELECT_BEGIN), K_ED(E_MOVE_LEFT)},
    {K_ESC, K_RIGHT},           {K_ED(E_SELECT_BEGIN), K_ED(E_MOVE_RIGHT)},
    {K_ESC, K_UP},              {K_ED(E_SELECT_BEGIN), K_ED(E_MOVE_UP)},
    {K_ESC, K_DOWN},            {K_ED(E_SELECT_BEGIN), K_ED(E_MOVE_DOWN)},
    {K_ESC, K_ESC},             {K_ED(E_SELECT_END)},
    {K_LEFT | K_SHIFT},         {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_LEFT)},
    {K_RIGHT | K_SHIFT},        {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_RIGHT)},
    {K_UP | K_SHIFT},           {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_UP)},
    {K_DOWN | K_SHIFT},         {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_DOWN)},
    {K_HOME | K_SHIFT},         {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_STR_BEGIN)},
    {K_END | K_SHIFT},          {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_STR_END)},
    {K_PAGEUP | K_SHIFT},       {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_PAGE_UP)},
    {K_PAGEDN | K_SHIFT},       {K_ED(E_SELECT_BEGIN) + 1, K_ED(E_MOVE_PAGE_DOWN)},
    {K_RELEASE | K_SHIFT},      {K_ED(E_SELECT_END) + 1},
    {K_F4},                     {K_ED(E_SELECT_MODE)},
    {K_F4 | K_SHIFT},           {K_ED(E_SELECT_MODE) + 1},
    {K_F8},                     {K_ED(E_SELECT_UNSELECT)},

    {K_F5},                     {K_ED(E_EDIT_BLOCK_COPY)},
    {K_F6},                     {K_ED(E_EDIT_BLOCK_MOVE)},
    {K_F8 | K_SHIFT},           {K_ED(E_EDIT_BLOCK_DEL)},
    {'Y' | K_ALT},              {K_ED(E_EDIT_BLOCK_DEL)},
    {K_ESC, K_F8},              {K_ED(E_EDIT_BLOCK_DEL)},
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
    {'R' | K_CTRL},             {K_ED(E_DLG_REPLACE)},
    {'H' | K_CTRL},             {K_ED(E_DLG_REPLACE)},

    {K_F7 | K_CTRL},            {K_ED(E_CTRL_FIND)},
    {K_F7 | K_SHIFT},           {K_ED(E_CTRL_REPEAT)},
    {'P' | K_CTRL},             {K_ED(E_CTRL_FINDUP)},
    {'N' | K_CTRL},             {K_ED(E_CTRL_FINDDN)},
    {'P' | K_ALT},              {K_ED(E_CTRL_FINDUPW)},
    {'N' | K_ALT},              {K_ED(E_CTRL_FINDDNW)},

    {'J' | K_CTRL},             {K_ED(E_CTRL_FLIST)},
    {'A' | K_CTRL},             {K_ED(E_CTRL_PROPERTIES)},
    {'M' | K_CTRL},             {K_ED(E_CTRL_CHANGE_CP)},

    {'S' | K_CTRL},             {K_ED(E_CTRL_SAVE)},
    {K_ESC, K_F2},              {K_ED(E_CTRL_SAVE) | 1},
    {K_F2 | K_SHIFT},           {K_ED(E_CTRL_SAVEAS)},
    {K_ESC, K_F3},              {K_ED(E_CTRL_RELOAD)},
    {K_F10 | K_SHIFT},          {K_ED(E_CTRL_CLOSE)},
    {K_ESC, K_F10},             {K_ED(E_CTRL_CLOSE)},

    {'M' | K_ALT},              {K_ED(E_POPUP_MENU)},
    {K_ESC, 'm'},               {K_ED(E_POPUP_MENU)},
    {K_ESC, 'M'},               {K_ED(E_POPUP_MENU)},

    //for testing
    { 'T' | K_ALT },            {K_ED(E_CTRL_REFRESH)}
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
    {E_CTRL_FLIST,          {&EditorWnd::CtrlFuncList,           EditorWnd::select_state::end}},
    {E_CTRL_PROPERTIES,     {&EditorWnd::CtrlProperties,         EditorWnd::select_state::end}},
    {E_CTRL_CHANGE_CP,      {&EditorWnd::CtrlChangeCP,           EditorWnd::select_state::no}},
    {E_POPUP_MENU,          {&EditorWnd::TrackPopupMenu,         EditorWnd::select_state::end}}
};

