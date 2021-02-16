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
    {K_F7 | K_SHIFT},           {K_ED(E_CTRL_AGAIN)},
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
    {K_ESC, 'M'},               {K_ED(E_POPUP_MENU)}
};

#if 0
//////////////////////////////////////////////////////////////////////////////
int g_aiAppKeyMap[] = {
  K_F1, 0,                   K_APP_HELP, 0,
//  K_ESC, K_F1, 0,            K_APP_ABOUT, 0,

  'S'  | K_ALT, 0,           K_APP_SAVE_ALL, 0,
  K_F2, 0,                   K_APP_SAVE_ALL, 0,

  K_F3, 0,                   K_APP_DLG_OPEN, 0,
  K_F3 | K_SHIFT, 0,         K_APP_DLG_LOAD, 0,

  K_F5 | K_SHIFT, 0,         K_APP_WND_COPY, 0,
  K_F6 | K_SHIFT, 0,         K_APP_WND_MOVE, 0,

  K_ESC, K_ENTER, 0,         K_APP_OPEN, 0,
  K_ESC, K_SPACE, 0,         K_APP_EXEC, 0,

  'F' | K_ALT, 0,            K_APP_FINDFILE, 0,
  K_ESC, K_F7, 0,            K_APP_FINDFILE, 0,
  K_ESC, 'f', 0,             K_APP_FINDFILE, 0,
  K_ESC, 'F', 0,             K_APP_FINDFILE, 0,
  'R' | K_ALT, 0,            K_APP_REPLACEFILE, 0,
  'H' | K_ALT, 0,            K_APP_REPLACEFILE, 0,
  K_ESC, 'r', 0,             K_APP_REPLACEFILE, 0,
  K_ESC, 'R', 0,             K_APP_REPLACEFILE, 0,

  'L' | K_CTRL, 0,           K_APP_FOUNDFILE, 0,
  'L' | K_ALT, 0,            K_APP_FOUNDFILE, 0,

  'W' | K_CTRL, 0,           K_APP_WND_LIST, 0,

  K_F9, 0,                   K_MENU, 0,
  K_ESC, K_F1, 0,            K_MENU, 0,

  K_F10, 0,                  K_EXIT, 0,
  'X' | K_ALT, 0,            K_EXIT, 0,
  K_ESC, 'x', 0,             K_EXIT, 0,
  K_ESC, 'X', 0,             K_EXIT, 0,

  K_ESC, '0', 0,             K_INSERT, 0,

  '/' | K_ALT, 0,            K_APP_VIEW, 0,
  '+' | K_ALT, 0,            K_APP_VIEW_MODE, 0,
  '=' | K_ALT, 0,            K_APP_VIEW_MODE, 0,
  '-' | K_ALT, 0,            K_APP_VIEW_SIZE, 0,
  K_ESC, '/', 0,             K_APP_VIEW, 0,
  K_ESC, '=', 0,             K_APP_VIEW_MODE, 0,
  K_ESC, '+', 0,             K_APP_VIEW_MODE, 0,
  K_ESC, '-', 0,             K_APP_VIEW_SIZE, 0,
  K_PAGEUP | K_ALT, 0,       K_APP_VIEW_SET, 0,
  K_PAGEDN | K_ALT, 0,       K_APP_VIEW_SET, 0,
  K_ESC, K_PAGEUP, 0,        K_APP_VIEW_SET, 0,
  K_ESC, K_PAGEDN, 0,        K_APP_VIEW_SET, 0,
  K_ESC, '9', 0,             K_APP_VIEW_SET, 0,
  K_ESC, '3', 0,             K_APP_VIEW_SET, 0,

  'D' | K_CTRL, 0,           K_APP_DIFF, 0,
  '\\' | K_CTRL, 0,          K_APP_RACCESS, 0,
  '\\' | K_ALT, 0,           K_APP_RACCESS, 0,

  '0' | K_ALT, 0,            K_APP_RACCESS_0, 0,
  '1' | K_ALT, 0,            K_APP_RACCESS_1, 0,
  '2' | K_ALT, 0,            K_APP_RACCESS_2, 0,
  '3' | K_ALT, 0,            K_APP_RACCESS_3, 0,
  '4' | K_ALT, 0,            K_APP_RACCESS_4, 0,
  '5' | K_ALT, 0,            K_APP_RACCESS_5, 0,
  '6' | K_ALT, 0,            K_APP_RACCESS_6, 0,
  '7' | K_ALT, 0,            K_APP_RACCESS_7, 0,
  '8' | K_ALT, 0,            K_APP_RACCESS_8, 0,
  '9' | K_ALT, 0,            K_APP_RACCESS_9, 0,

  'K' | K_CTRL, 0,           K_APP_RECORD_MACRO, 0,
  'K' | K_ALT, 0,            K_APP_PLAY_MACRO, 0,
  'Q' | K_CTRL, 0,           K_APP_KEYGEN, 0,

  //for test
  K_F12, 0,                  K_REFRESH, 0,
  't' | K_ALT, 0,            K_REFRESH, 0,

  0, 0
};


int* g_pAppKeyMap = g_aiAppKeyMap;



int* g_pEditKeyMap = g_aiEditKeyMap;

//////////////////////////////////////////////////////////////////////////////
int KeySectionApp(int mode);
int KeySectionEdit(int mode);

static char* s_pKey = "";

static DVar KMapParam[] = {
  {"Key", VAR_STR | VAR_PROC, &s_pKey},
  {0}
};

static DSection KMapFile[] = {
  {"AppKeyMap",     KMapParam, KeySectionApp},
  {"WndEditKeyMap", KMapParam, KeySectionEdit},
  {0}
};

static char* pKMapCfgRem = ";" EDITOR_NAME " key mapping cfg file\n";


//////////////////////////////////////////////////////////////////////////////
KWD Keys[] = {
  //Keys
  {"Shift",                     K_SHIFT},
  {"Ctrl",                      K_CTRL},
  {"Alt",                       K_ALT},

  {"F1",                        K_F1},
  {"F2",                        K_F2},
  {"F3",                        K_F3},
  {"F4",                        K_F4},
  {"F5",                        K_F5},
  {"F6",                        K_F6},
  {"F7",                        K_F7},
  {"F8",                        K_F8},
  {"F9",                        K_F9},
  {"F10",                       K_F10},
  {"F11",                       K_F11},
  {"F12",                       K_F12},

  {"Esc",                       K_ESC},
  {"Enter",                     K_ENTER},
  {"Space",                     K_SPACE},
  {"BackSpace",                 K_BS},
  {"Tab",                       K_TAB},

  {"Insert",                    K_INSERT},
  {"Delete",                    K_DELETE},
  {"Home",                      K_HOME},
  {"End",                       K_END},
  {"PageUp",                    K_PAGEUP},
  {"PageDn",                    K_PAGEDN},

  {"Up",                        K_UP},
  {"Down",                      K_DOWN},
  {"Left",                      K_LEFT},
  {"Right",                     K_RIGHT},

  {"MouseScrollUp",             K_MOUSEWUP | K_MOUSEW},
  {"MouseScrollDown",           K_MOUSEWDN | K_MOUSEW},
  {"MouseLeft2",                K_MOUSEKL  | K_MOUSE2},
  {"MouseLeft3",                K_MOUSEKL  | K_MOUSE3},

  {"ShiftRelease",              K_RELEASE  | K_SHIFT},

  //App
  {"APP_EXIT",                  K_EXIT},
  {"APP_MENU",                  K_MENU},
  {"APP_REFRESH",               K_REFRESH},
  {"APP_INSERT_MODE",           K_INSERT},

  {"APP_ABOUT",                 K_APP_ABOUT},
  {"APP_HELP",                  K_APP_HELP},

  {"APP_NEW",                   K_APP_NEW},
  {"APP_GETSUBSTR_OPEN",        K_APP_OPEN},
  {"APP_GETSUBSTR_EXEC",        K_APP_EXEC},
  {"APP_SAVE_ALL",              K_APP_SAVE_ALL},
  {"APP_DLG_OPEN",              K_APP_DLG_OPEN},
  {"APP_DLG_LOAD",              K_APP_DLG_LOAD},
  {"APP_DLG_EXEC",              K_APP_DLG_EXEC},

  {"APP_DLG_FINDFILE",          K_APP_FINDFILE},
  {"APP_DLG_REPLACEFILE",       K_APP_REPLACEFILE},
  {"APP_DLG_FOUNDFILE",         K_APP_FOUNDFILE},
  {"APP_DLG_WND_COPY",          K_APP_WND_COPY},
  {"APP_DLG_WND_MOVE",          K_APP_WND_MOVE},

  {"APP_WND_CLOSE",             K_APP_WND_CLOSE},
  {"APP_WND_CLOSEALL",          K_APP_WND_CLOSEALL},
  {"APP_WND_NEXT",              K_APP_WND_NEXT},
  {"APP_WND_PREV",              K_APP_WND_PREV},
  {"APP_DLG_WND_LIST",          K_APP_WND_LIST},

  {"APP_VIEW_SPLIT_MERGE",      K_APP_VIEW},
  {"APP_VIEW_SPLIT_VH",         K_APP_VIEW_MODE},
  {"APP_VIEW_SELECT",           K_APP_VIEW_SET},
  {"APP_VIEW_SIZE",             K_APP_VIEW_SIZE},

  {"APP_DLG_DIFF",              K_APP_DIFF},
  {"APP_DLG_RND_ACCESS",        K_APP_RACCESS},
  {"APP_DLG_KEYGEN",            K_APP_KEYGEN},
  {"APP_DLG_NEW_SESSION",       K_APP_NEW_SESSION},
  {"APP_DLG_OPEN_SESSION",      K_APP_OPEN_SESSION},

  {"APP_RND_ACCESS_0",          K_APP_RACCESS_0},
  {"APP_RND_ACCESS_1",          K_APP_RACCESS_1},
  {"APP_RND_ACCESS_2",          K_APP_RACCESS_2},
  {"APP_RND_ACCESS_3",          K_APP_RACCESS_3},
  {"APP_RND_ACCESS_4",          K_APP_RACCESS_4},
  {"APP_RND_ACCESS_5",          K_APP_RACCESS_5},
  {"APP_RND_ACCESS_6",          K_APP_RACCESS_6},
  {"APP_RND_ACCESS_7",          K_APP_RACCESS_7},
  {"APP_RND_ACCESS_8",          K_APP_RACCESS_8},
  {"APP_RND_ACCESS_9",          K_APP_RACCESS_9},

  {"APP_RECORD_MACRO",          K_APP_RECORD_MACRO},
  {"APP_PLAY_MACRO",            K_APP_PLAY_MACRO},

  {"APP_COLOR",                 K_APP_COLOR},
  {"APP_SETTINGS",              K_APP_SETTINGS},

  //Editor wnd
  {"EDIT_MOVE_LEFT",            K_ED(E_MOVE_LEFT)},
  {"EDIT_MOVE_RIGHT",           K_ED(E_MOVE_RIGHT)},
  {"EDIT_MOVE_SCROLL_LEFT",     K_ED(E_MOVE_S_LEFT)},
  {"EDIT_MOVE_SCROLL_RIGHT",    K_ED(E_MOVE_S_RIGHT)},
  {"EDIT_MOVE_UP",              K_ED(E_MOVE_UP)},
  {"EDIT_MOVE_DOWN",            K_ED(E_MOVE_DOWN)},
  {"EDIT_MOVE_PAGE_UP",         K_ED(E_MOVE_PAGE_UP)},
  {"EDIT_MOVE_PAGE_DOWN",       K_ED(E_MOVE_PAGE_DOWN)},
  {"EDIT_MOVE_FILE_BEGIN",      K_ED(E_MOVE_FILE_BEGIN)},
  {"EDIT_MOVE_FILE_END",        K_ED(E_MOVE_FILE_END)},
  {"EDIT_MOVE_STR_BEGIN",       K_ED(E_MOVE_STR_BEGIN)},
  {"EDIT_MOVE_STR_END",         K_ED(E_MOVE_STR_END)},
  {"EDIT_MOVE_TAB_LEFT",        K_ED(E_MOVE_TAB_LEFT)},
  {"EDIT_MOVE_TAB_RIGHT",       K_ED(E_MOVE_TAB_RIGHT)},
  {"EDIT_MOVE_WORD_LEFT",       K_ED(E_MOVE_WORD_LEFT)},
  {"EDIT_MOVE_WORD_RIGHT",      K_ED(E_MOVE_WORD_RIGHT)},
  {"EDIT_MOVE_POS",             K_ED(E_MOVE_POS)},
  {"EDIT_MOVE_CENTER",          K_ED(E_MOVE_CENTER)},

  {"EDIT_SELECT_WORD",          K_ED(E_SELECT_WORD)},
  {"EDIT_SELECT_LINE",          K_ED(E_SELECT_LINE)},
  {"EDIT_SELECT_ALL",           K_ED(E_SELECT_ALL)},
  {"EDIT_SELECT_STREAM_BEGIN",  K_ED(E_SELECT_BEGIN)},
  {"EDIT_SELECT_END",           K_ED(E_SELECT_END)},
  {"EDIT_SELECT_UNSELECT",      K_ED(E_SELECT_UNSELECT)},
  {"EDIT_SELECT_LINES_BEGIN",   K_ED(E_SELECT_MODE)},

  //{"EDIT_CHAR",                 K_ED(E_EDIT_C)},
  {"EDIT_DEL_CHAR",             K_ED(E_EDIT_DEL_C)},
  {"EDIT_BS",                   K_ED(E_EDIT_BS)},
  {"EDIT_TAB",                  K_ED(E_EDIT_TAB)},
  {"EDIT_ENTER",                K_ED(E_EDIT_ENTER)},
  {"EDIT_DEL_STR",              K_ED(E_EDIT_DEL_STR)},
  {"EDIT_DEL_BEGIN",            K_ED(E_EDIT_DEL_BEGIN)},
  {"EDIT_DEL_END",              K_ED(E_EDIT_DEL_END)},
  {"EDIT_BLOCK_CLEAR",          K_ED(E_EDIT_BLOCK_CLEAR)},
  {"EDIT_BLOCK_COPY",           K_ED(E_EDIT_BLOCK_COPY)},
  {"EDIT_BLOCK_MOVE",           K_ED(E_EDIT_BLOCK_MOVE)},
  {"EDIT_BLOCK_DEL",            K_ED(E_EDIT_BLOCK_DEL)},
  {"EDIT_BLOCK_INDENT",         K_ED(E_EDIT_BLOCK_INDENT)},
  {"EDIT_BLOCK_UNDENT",         K_ED(E_EDIT_BLOCK_UNDENT)},
  {"EDIT_CB_COPY",              K_ED(E_EDIT_CB_COPY)},
  {"EDIT_CB_CUT",               K_ED(E_EDIT_CB_CUT)},
  {"EDIT_CB_PASTE",             K_ED(E_EDIT_CB_PASTE)},
  {"EDIT_UNDO",                 K_ED(E_EDIT_UNDO)},
  {"EDIT_REDO",                 K_ED(E_EDIT_REDO)},

  {"EDIT_CTRL_DATA",            K_ED(E_CTRL_DATA)},
  {"EDIT_CTRL_GOTOX",           K_ED(E_CTRL_GOTOX)},
  {"EDIT_CTRL_GOTOY",           K_ED(E_CTRL_GOTOY)},
  {"EDIT_CTRL_FIND",            K_ED(E_CTRL_FIND)},
  {"EDIT_CTRL_FINDUP",          K_ED(E_CTRL_FINDUP)},
  {"EDIT_CTRL_FINDDN",          K_ED(E_CTRL_FINDDN)},
  {"EDIT_CTRL_FINDUPWORD",      K_ED(E_CTRL_FINDUPW)},
  {"EDIT_CTRL_FINDDNWORD",      K_ED(E_CTRL_FINDDNW)},
  {"EDIT_CTRL_REPLACE",         K_ED(E_CTRL_REPLACE)},
  {"EDIT_CTRL_AGAIN",           K_ED(E_CTRL_AGAIN)},
  {"EDIT_DLG_GOTO",             K_ED(E_DLG_GOTO)},
  {"EDIT_DLG_FIND",             K_ED(E_DLG_FIND)},
  {"EDIT_DLG_REPLACE",          K_ED(E_DLG_REPLACE)},

  {"EDIT_CTRL_GETSUBSTR",       K_ED(E_CTRL_GETSUBSTR)},
  {"EDIT_CTRL_REFRESH",         K_ED(E_CTRL_REFRESH)},
  {"EDIT_CTRL_RELOAD",          K_ED(E_CTRL_RELOAD)},
  {"EDIT_CTRL_SAVE",            K_ED(E_CTRL_SAVE)},
  {"EDIT_CTRL_SAVEAS",          K_ED(E_CTRL_SAVEAS)},
  {"EDIT_CTRL_CLOSE",           K_ED(E_CTRL_CLOSE)},

  {"EDIT_MOVE_BRACKET_MATCH",   K_ED(E_MOVE_LEX_MATCH)},
  {"EDIT_DLG_FUNCLIST",         K_ED(E_CTRL_FLIST)},
  {"EDIT_DLG_PROPERTIES",       K_ED(E_CTRL_PROPERTIES)},
  {"EDIT_CTRL_CHANGE_CP",       K_ED(E_CTRL_CHANGE_CP)},
  {"EDIT_POPUP_MENU",           K_ED(E_POPUP_MENU)},

  //Editor additional
  {"EDIT_MOVE_SCROLL_UP",       K_ED(E_MOVE_UP) + 1},
  {"EDIT_MOVE_SCROLL_DOWN",     K_ED(E_MOVE_DOWN) + 1},
  {"EDIT_SELECT_SHIFT_BEGIN",   K_ED(E_SELECT_BEGIN) + 1},
  {"EDIT_SELECT_SHIFT_END",     K_ED(E_SELECT_END) + 1},
  {"EDIT_SELECT_COLUMN_BEGIN",  K_ED(E_SELECT_MODE) + 1},
  {"EDIT_CTRL_OVERWRITE",       K_ED(E_CTRL_SAVE) + 1},
  {"EDIT_MOVE_SCROLL3_UP",      K_ED(E_MOVE_UP) + 3},
  {"EDIT_MOVE_SCROLL3_DOWN",    K_ED(E_MOVE_DOWN) + 3},

  {0}
};


KWList KWMap(Keys);

KeyMap AppKeys(g_aiAppKeyMap);
KeyMap EditKeys(g_aiEditKeyMap);


//////////////////////////////////////////////////////////////////////////////
KWList::KWList(KWD* pKWD)
{
  m_pKWord = NULL;
  while(pKWD->pName)
  {
    KWord* pKWord = new KWord(pKWD->pName, pKWD->val);
    if(pKWord)
      Var::_Add((Var**)&m_pKWord, pKWord);

    ++pKWD;
  }
}


KWList::~KWList()
{
  if(m_pKWord)
    delete m_pKWord;
}


int KWList::GetVal(char* pName)
{
  Var* pVar = m_pKWord->Find(pName);
  if(!pVar)
    return 0;
  else
    return ((KWord*) pVar)->GetVal();
}


const char* KWList::GetName(int val)
{
  Var* pVar = m_pKWord;
  while(pVar)
  {
    if(((KWord*) pVar)->GetVal() == val)
      return pVar->GetName();
    pVar = pVar->m_pNext;
  }

  return NULL;
}


//////////////////////////////////////////////////////////////////////////////
int KeySection(KeyMap* pKMap)
{
  //буфер разобранных значений
  int KBuff[32];
  memset(KBuff, 0, sizeof(KBuff));
  int pos   = 0;

  //переменные для проверки порядка данных
  int check = 0;
  int cmd   = 1;
  int add   = 0;

  char* pStr = s_pKey;
  while(*pStr)
  {
    while(*pStr == ' ')
      ++pStr;

    int  len = 0;
    char key[64];
    char c;
    while((c = *pStr) != 0)
    {
      if(c == ' ' || c == '+' || c == ':')
        break;

      key[len++] = c;
      ++pStr;
    }
    key[len] = 0;

    if(!len && (c == '+' || c == ':'))
    {
      ++pStr;

      if(add)
      {
        int val = '+';

        //TPRINT(("Key=%s v=%x\n", key, val));
        if(c == '+')
          //идет + +
          KBuff[pos++] |= val;
        else
        {
          //идет + :
          KBuff[++pos] |= val;
          ++pos;
        }
        check |= cmd;
        add = 0;
      }
      else if(c == '+')
      {
        --pos;
        add = 1;
      }

      //: всегда разделитель
      if(c == ':')
      {
        ++pos;
        cmd <<= 1;
      }
    }
    else if(len)
    {
      int val;
      if(len == 1)
        val = key[0];//symbol
      else
        val = KWMap.GetVal(key);

      if(!val)
        //error not found
        return 0;

      //TPRINT(("Key=%s v=%x\n", key, val));
      KBuff[pos++] |= val;
      check |= cmd;
      add = 0;
    }
  }

  if(!add && check == 3)
  {
    //разбор нормальный
    //TDUMP((KBuff, pos * 4 + 4, "Key"));
    pKMap->AddKey(KBuff, pos + 1);
  }

  return 0;
}


int KeySectionApp(int mode)
{
  //TPRINT(("KeySectionApp val=%s\n", s_pKey));

  if(mode != 0)//scan
    return 0;
  return KeySection(&AppKeys);
}


int KeySectionEdit(int mode)
{
  //TPRINT(("KeySectionEdit val=%s\n", s_pKey));

  if(mode != 0)//scan
    return 0;
  return KeySection(&EditKeys);
}


//////////////////////////////////////////////////////////////////////////////
int LoadKMap(char* pFile)
{
  //TPRINT(("LoadKMap %s\n", pFile));

  IniFile iKMapCfgFile(KMapFile);
  iKMapCfgFile.Load(pFile);

  return 0;
}


int SaveKey(FSave* pFile, int* pKey)
{
  int k;
  while((k = *pKey) != 0)
  {
    char str[256];
    strcpy_s(str, sizeof(str), "Key = \"");

    while((k = *pKey) != 0)
    {
      int type = k;

      const char* pKCod = KWMap.GetName(type);
      if(pKCod)
      {
        strcat_s(str, sizeof(str), pKCod);
        //TPRINT(("Key full=%s\n", pKCod));
      }
      else
      {
        if(k & K_SHIFT)
          strcat_s(str, sizeof(str), "Shift+");
        if(k & K_CTRL)
          strcat_s(str, sizeof(str), "Ctrl+");
        if(k & K_ALT)
          strcat_s(str, sizeof(str), "Alt+");

        type = k & K_TYPEMASK;
        if((pKCod = KWMap.GetName(type)) != NULL)
        {
          strcat_s(str, sizeof(str), pKCod);
          //TPRINT(("Key type=%s\n", pKCod));
        }
        else
        {
          type = k & K_CODEMASK;
          if((pKCod = KWMap.GetName(type)) != NULL)
          {
            //TPRINT(("Key code=%s\n", pKCod));
            strcat_s(str, sizeof(str), pKCod);
          }
          else
          {
            //TPRINT(("Key code=%c\n", type));
            char s[2];
            s[0] = (char)type;
            s[1] = 0;
            strcat_s(str, sizeof(str), s);
          }
        }
      }
      ++pKey;
      strcat_s(str, sizeof(str), " ");
    }
    ++pKey;

    char* pSplit = "                         :";
    size_t l = strlen(str);
    if(l < strlen(pSplit))
      strcat_s(str, sizeof(str), pSplit + l);
    else
      strcat_s(str, sizeof(str), ":");

    //TPRINT(("Key=%s\n", str));

    while((k = *pKey) != 0)
    {
      strcat_s(str, sizeof(str), " ");

      int type = k;

      const char* pKCod = KWMap.GetName(type);
      if(pKCod)
      {
        //TPRINT(("code full=%s\n", pKCod));
        strcat_s(str, sizeof(str), pKCod);
      }
      else
      {
        type = k & 0xffff0000;
        if((pKCod = KWMap.GetName(type)) != NULL)
        {
          //TPRINT(("code type=%s\n", pKCod));
          strcat_s(str, sizeof(str), pKCod);
          if(k & K_CODEMASK)
          {
            TPRINT(("Code mask=%x\n", k & K_CODEMASK));
          }
        }
        else
        {
          TPRINT(("ERROR code=%x\n", k));
        }
      }
      ++pKey;
    }
    ++pKey;

    //TPRINT(("%s\n", str));
    pFile->fprint("%s\"\n", str);
  }

  return 0;
}


int SaveKMap(char* pFile)
{
  TPRINT(("SaveKMap %s\n", pFile));

  FSave fSave(pFile);
  fSave.fprint(pKMapCfgRem);

  fSave.fprint("\n[AppKeyMap]\n");
  SaveKey(&fSave, AppKeys.GetMap());

  fSave.fprint("\n[WndEditKeyMap]\n");
  SaveKey(&fSave, EditKeys.GetMap());

  return 0;
}


int* GetKeySeq(int code)
{
  int* pSeq = AppKeys.GetKeySeq(code);
  if(!pSeq)
    pSeq = EditKeys.GetKeySeq(code);

  return pSeq;
}


const char* GetKeyName(int code)
{
  static char buff[64];
  buff[0] = 0;

  int* pSeq = GetKeySeq(code);
  if(!pSeq)
    return "";

  while(*pSeq)
  {
    strcat_s(buff, sizeof(buff), " ");

    int k = *pSeq++;

    if(k & K_SHIFT)
      strcat_s(buff, sizeof(buff), "Shift+");
    if(k & K_CTRL)
      strcat_s(buff, sizeof(buff), "Ctrl+");
    if(k & K_ALT)
      strcat_s(buff, sizeof(buff), "Alt+");

    const char* pKCod = KWMap.GetName(k & K_TYPEMASK);
    if(pKCod)
    {
      strcat_s(buff, sizeof(buff), pKCod);
    }
    else
    {
      k &= K_CODEMASK;
      if((pKCod = KWMap.GetName(k)) != NULL)
      {
        //TPRINT(("Key code=%s\n", pKCod));
        strcat_s(buff, sizeof(buff), pKCod);
      }
      else
      {
        //TPRINT(("Key code=%c\n", type));
        char s[2];
        s[0] = (char)k;
        s[1] = 0;
        strcat_s(buff, sizeof(buff), s);
      }
    }
  }

  return buff;
}

#endif