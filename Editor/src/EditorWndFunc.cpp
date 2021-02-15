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

#include "EditorWnd.h"
#include "EditorCmd.h"


std::unordered_map<EditorCmd, std::pair<EditorWnd::EditorFunc, EditorWnd::Select>> EditorWnd::s_funcMap =
{
    {E_MOVE_LEFT,           {&EditorWnd::MoveLeft,               EditorWnd::Select::begin}},
    {E_MOVE_RIGHT,          {&EditorWnd::MoveRight,              EditorWnd::Select::begin}},
    {E_MOVE_S_LEFT,         {&EditorWnd::MoveScrollLeft,         EditorWnd::Select::begin}},
    {E_MOVE_S_RIGHT,        {&EditorWnd::MoveScrollRight,        EditorWnd::Select::begin}},
    {E_MOVE_UP,             {&EditorWnd::MoveUp,                 EditorWnd::Select::begin}},
    {E_MOVE_DOWN,           {&EditorWnd::MoveDown,               EditorWnd::Select::begin}},
    {E_MOVE_PAGE_UP,        {&EditorWnd::MovePageUp,             EditorWnd::Select::begin}},
    {E_MOVE_PAGE_DOWN,      {&EditorWnd::MovePageDown,           EditorWnd::Select::begin}},
    {E_MOVE_FILE_BEGIN,     {&EditorWnd::MoveFileBegin,          EditorWnd::Select::begin}},
    {E_MOVE_FILE_END,       {&EditorWnd::MoveFileEnd,            EditorWnd::Select::begin}},
    {E_MOVE_STR_BEGIN,      {&EditorWnd::MoveStrBegin,           EditorWnd::Select::begin}},
    {E_MOVE_STR_END,        {&EditorWnd::MoveStrEnd,             EditorWnd::Select::begin}},
    {E_MOVE_TAB_LEFT,       {&EditorWnd::MoveTabLeft,            EditorWnd::Select::begin}},
    {E_MOVE_TAB_RIGHT,      {&EditorWnd::MoveTabRight,           EditorWnd::Select::begin}},
    {E_MOVE_WORD_LEFT,      {&EditorWnd::MoveWordLeft,           EditorWnd::Select::begin}},
    {E_MOVE_WORD_RIGHT,     {&EditorWnd::MoveWordRight,          EditorWnd::Select::begin}},
    {E_MOVE_POS,            {&EditorWnd::MovePos,                EditorWnd::Select::begin}},
    {E_MOVE_CENTER,         {&EditorWnd::MoveCenter,             EditorWnd::Select::no}},

    {E_SELECT_WORD,         {&EditorWnd::SelectWord,             EditorWnd::Select::no}},
    {E_SELECT_LINE,         {&EditorWnd::SelectLine,             EditorWnd::Select::no}},
    {E_SELECT_ALL,          {&EditorWnd::SelectAll,              EditorWnd::Select::no}},
    {E_SELECT_BEGIN,        {&EditorWnd::SelectBegin,            EditorWnd::Select::no}},
    {E_SELECT_END,          {&EditorWnd::SelectEnd,              EditorWnd::Select::no}},
    {E_SELECT_UNSELECT,     {&EditorWnd::SelectUnselect,         EditorWnd::Select::no}},
    {E_SELECT_MODE,         {&EditorWnd::SelectMode,             EditorWnd::Select::no}},

    {E_EDIT_C,              {&EditorWnd::EditC,                  EditorWnd::Select::end}},
    {E_EDIT_DEL_C,          {&EditorWnd::EditDelC,               EditorWnd::Select::end}},
    {E_EDIT_BS,             {&EditorWnd::EditBS,                 EditorWnd::Select::end}},
    {E_EDIT_TAB,            {&EditorWnd::EditTab,                EditorWnd::Select::end}},
    {E_EDIT_ENTER,          {&EditorWnd::EditEnter,              EditorWnd::Select::end}},
    {E_EDIT_DEL_STR,        {&EditorWnd::EditDelStr,             EditorWnd::Select::end}},
    {E_EDIT_DEL_BEGIN,      {&EditorWnd::EditDelBegin,           EditorWnd::Select::end}},
    {E_EDIT_DEL_END,        {&EditorWnd::EditDelEnd,             EditorWnd::Select::end}},

    {E_EDIT_BLOCK_CLEAR,    {&EditorWnd::EditBlockClear,         EditorWnd::Select::end}},
    {E_EDIT_BLOCK_COPY,     {&EditorWnd::EditBlockCopy,          EditorWnd::Select::end}},
    {E_EDIT_BLOCK_MOVE,     {&EditorWnd::EditBlockMove,          EditorWnd::Select::end}},
    {E_EDIT_BLOCK_DEL,      {&EditorWnd::EditBlockDel,           EditorWnd::Select::end}},
    {E_EDIT_BLOCK_INDENT,   {&EditorWnd::EditBlockIndent,        EditorWnd::Select::end}},
    {E_EDIT_BLOCK_UNDENT,   {&EditorWnd::EditBlockUndent,        EditorWnd::Select::end}},
    {E_EDIT_CB_COPY,        {&EditorWnd::EditCopyToClipboard,    EditorWnd::Select::end}},
    {E_EDIT_CB_CUT,         {&EditorWnd::EditCutToClipboard,     EditorWnd::Select::end}},
    {E_EDIT_CB_PASTE,       {&EditorWnd::EditPasteFromClipboard, EditorWnd::Select::end}},
    {E_EDIT_UNDO,           {&EditorWnd::EditUndo,               EditorWnd::Select::end}},
    {E_EDIT_REDO,           {&EditorWnd::EditRedo,               EditorWnd::Select::end}},

    {E_CTRL_DATA,           {&EditorWnd::Data,                   EditorWnd::Select::no}},
    {E_CTRL_GOTOX,          {&EditorWnd::GotoX,                  EditorWnd::Select::begin}},
    {E_CTRL_GOTOY,          {&EditorWnd::GotoY,                  EditorWnd::Select::begin}},
    {E_CTRL_FIND,           {&EditorWnd::CtrlFind,               EditorWnd::Select::begin}},
    {E_CTRL_FINDUP,         {&EditorWnd::CtrlFindUp,             EditorWnd::Select::begin}},
    {E_CTRL_FINDDN,         {&EditorWnd::CtrlFindDown,           EditorWnd::Select::begin}},
    {E_CTRL_FINDUPW,        {&EditorWnd::FindUpWord,             EditorWnd::Select::begin}},
    {E_CTRL_FINDDNW,        {&EditorWnd::FindDownWord,           EditorWnd::Select::begin}},
    {E_CTRL_REPLACE,        {&EditorWnd::Replace,                EditorWnd::Select::end}},
    {E_CTRL_AGAIN,          {&EditorWnd::Again,                  EditorWnd::Select::end}},

    {E_DLG_GOTO,            {&EditorWnd::DlgGoto,                EditorWnd::Select::end}},
    {E_DLG_FIND,            {&EditorWnd::DlgFind,                EditorWnd::Select::end}},
    {E_DLG_REPLACE,         {&EditorWnd::DlgReplace,             EditorWnd::Select::end}},

    {E_CTRL_GETSUBSTR,      {&EditorWnd::CtrlGetSubstr,          EditorWnd::Select::end}},
    {E_CTRL_REFRESH,        {&EditorWnd::CtrlRefresh,            EditorWnd::Select::end}},
    {E_CTRL_RELOAD,         {&EditorWnd::Reload,                 EditorWnd::Select::end}},
    {E_CTRL_SAVE,           {&EditorWnd::Save,                   EditorWnd::Select::end}},
    {E_CTRL_SAVEAS,         {&EditorWnd::SaveAs,                 EditorWnd::Select::end}},
    {E_CTRL_CLOSE,          {&EditorWnd::Close,                  EditorWnd::Select::end}},

    {E_MOVE_LEX_MATCH,      {&EditorWnd::MoveLexMatch,           EditorWnd::Select::begin}},
    {E_CTRL_FLIST,          {&EditorWnd::CtrlFList,              EditorWnd::Select::end}},
    {E_CTRL_PROPERTIES,     {&EditorWnd::CtrlProperties,         EditorWnd::Select::end}},
    {E_CTRL_CHANGE_CP,      {&EditorWnd::CtrlChangeCP,           EditorWnd::Select::no}},
    {E_POPUP_MENU,          {&EditorWnd::TrackPopupMenu,         EditorWnd::Select::end}}
};

bool EditorWnd::MovePos(input_t cmd)
{
    pos_t x = K_GET_X(cmd);
    pos_t y = K_GET_Y(cmd);

    if (x < m_sizeX && y < m_sizeY)
    {
        m_cursorx = x;
        m_cursory = y;
    }
    
    return true;
}

bool EditorWnd::MoveLeft(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveRight(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveScrollLeft(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveScrollRight(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveUp(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveDown(input_t cmd)
{
    return true;
}

bool EditorWnd::MovePageUp(input_t cmd)
{
    return true;
}

bool EditorWnd::MovePageDown(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveFileBegin(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveFileEnd(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveStrBegin(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveStrEnd(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveTabLeft(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveTabRight(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveWordLeft(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveWordRight(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveCenter(input_t cmd)
{
    return true;
}

bool EditorWnd::SelectWord(input_t cmd)
{
    return true;
}

bool EditorWnd::SelectLine(input_t cmd)
{
    return true;
}

bool EditorWnd::SelectAll(input_t cmd)
{
    return true;
}

bool EditorWnd::SelectBegin(input_t cmd)
{
    return true;
}

bool EditorWnd::SelectEnd(input_t cmd)
{
    return true;
}

bool EditorWnd::SelectUnselect(input_t cmd)
{
    return true;
}

bool EditorWnd::SelectMode(input_t cmd)
{
    return true;
}

bool EditorWnd::EditC(input_t cmd)
{
    return true;
}

bool EditorWnd::EditDelC(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBS(input_t cmd)
{
    return true;
}

bool EditorWnd::EditTab(input_t cmd)
{
    return true;
}

bool EditorWnd::EditEnter(input_t cmd)
{
    return true;
}

bool EditorWnd::EditDelStr(input_t cmd)
{
    return true;
}

bool EditorWnd::EditDelBegin(input_t cmd)
{
    return true;
}

bool EditorWnd::EditDelEnd(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockClear(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockCopy(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockMove(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockDel(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockIndent(input_t cmd)
{
    return true;
}

bool EditorWnd::EditBlockUndent(input_t cmd)
{
    return true;
}

bool EditorWnd::EditCopyToClipboard(input_t cmd)
{
    return true;
}

bool EditorWnd::EditCutToClipboard(input_t cmd)
{
    return true;
}

bool EditorWnd::EditPasteFromClipboard(input_t cmd)
{
    return true;
}

bool EditorWnd::EditUndo(input_t cmd)
{
    return true;
}

bool EditorWnd::EditRedo(input_t cmd)
{
    return true;
}

bool EditorWnd::Data(input_t cmd)
{
    return true;
}

bool EditorWnd::GotoX(input_t cmd)
{
    return true;
}

bool EditorWnd::GotoY(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFind(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFindUp(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFindDown(input_t cmd)
{
    return true;
}

bool EditorWnd::FindUpWord(input_t cmd)
{
    return true;
}

bool EditorWnd::FindDownWord(input_t cmd)
{
    return true;
}

bool EditorWnd::Replace(input_t cmd)
{
    return true;
}

bool EditorWnd::Again(input_t cmd)
{
    return true;
}

bool EditorWnd::DlgGoto(input_t cmd)
{
    return true;
}

bool EditorWnd::DlgFind(input_t cmd)
{
    return true;
}

bool EditorWnd::DlgReplace(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlGetSubstr(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlRefresh(input_t cmd)
{
    return true;
}

bool EditorWnd::Reload(input_t cmd)
{
    return true;
}

bool EditorWnd::Save(input_t cmd)
{
    return true;
}

bool EditorWnd::SaveAs(input_t cmd)
{
    return true;
}

bool EditorWnd::Close(input_t cmd)
{
    return true;
}

bool EditorWnd::MoveLexMatch(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlFList(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlProperties(input_t cmd)
{
    return true;
}

bool EditorWnd::CtrlChangeCP(input_t cmd)
{
    return true;
}

bool EditorWnd::TrackPopupMenu(input_t cmd)
{
    return true;
}

