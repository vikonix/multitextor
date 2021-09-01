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

namespace _Editor
{

/////////////////////////////////////////////////////////////////////////////
Logo g_logo 
{
  ColorScreen,
  ColorScreen,
  '-',
  -1, 
  -1, 
  {
      "----------  ----",
      "--------- # ----",
      "-------- ## ----",
      "------- ###   --",
      "------ #### # --",
      "----- #### ## --",
      "---- #### ###   ",
      "--- #### #### # ",
      "-- #### #### ## ",
      "- #### #### ### ",
      " #### #### #### "
  }
};

/////////////////////////////////////////////////////////////////////////////
//MAIN access menu
menu_list g_accessMenu
{
    {MENU_ITEM, "&F&1Help"},
    {MENU_ITEM, "&F&2Save"},
    {MENU_ITEM, "&F&3Open"},
    {MENU_ITEM, "&F&4Mark"},
    {MENU_ITEM, "&F&5Copy"},
    {MENU_ITEM, "&F&6Move"},
    {MENU_ITEM, "&F&7Search"},
    {MENU_ITEM, "&F&8Unmark"},
    {MENU_ITEM, "&F&9Menu"},
    {MENU_ITEM, "&F&1&0Exit"}
};

//REPLACE access menu
menu_list g_replaceMenu
{
    {MENU_ITEM, "&Prev"},
    {MENU_ITEM, "&Next"},
    {MENU_ITEM, "&All"},

    {MENU_ITEM, "& "},
    {MENU_ITEM, "&Quit Rep"},
    {MENU_ITEM, "lace"}
};

/////////////////////////////////////////////////////////////////////////////
static menu_list menuFile 
{
    {MENU_ITEM,         "&New",                         K_APP_NEW,                  "Open new empty file"},
    {MENU_ITEM,         "&Open...",                     K_APP_DLG_OPEN,             "Open file in new window"},
    {MENU_ITEM,         "&Reload",                      K_ED(E_CTRL_RELOAD),        "Reload current file"},
    {MENU_ITEM,         "&Close",                       K_ED(E_CTRL_CLOSE),         "Close current file"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "&Save",                        K_ED(E_CTRL_SAVE),          "Save current file"},
    {MENU_ITEM,         "Save &As...",                  K_ED(E_CTRL_SAVEAS),        "Save file as"},
    {MENU_ITEM,         "Save A&ll",                    K_APP_SAVE_ALL,             "Save all changed files"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "&Properties...",               K_ED(E_CTRL_PROPERTIES),    "Get file properties"},
    {MENU_SEPARATOR},
//    {MENU_ITEM,         "Ne&w Session...",              K_APP_NEW_SESSION,          "Create new session"},
//    {MENU_ITEM,         "O&pen Session...",             K_APP_OPEN_SESSION,         "Open existing session"},
//    {MENU_SEPARATOR},
    {MENU_ITEM,         "Recent &Files",                K_MENU + 9,                 "Recent files list"},
//    {MENU_ITEM,         "Recent Sess&ions",             K_MENU + 10,                "Recent sessions list"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "&Exit",                        K_EXIT,                     "Exit from editor"}
};

static menu_list menuEdit
{
    {MENU_ITEM,         "&Undo",                        K_ED(E_EDIT_UNDO),          "Undo the last action"},
    {MENU_ITEM,         "&Redo",                        K_ED(E_EDIT_REDO),          "Redo the previously undone action"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "Cu&t",                         K_ED(E_EDIT_CB_CUT),        "Cut selected text to clipboard"},
    {MENU_ITEM,         "&Copy",                        K_ED(E_EDIT_CB_COPY),       "Copy selected text to clipboard"},
    {MENU_ITEM,         "&Paste",                       K_ED(E_EDIT_CB_PASTE),      "Paste text from clipboard"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "Delete &Line",                 K_ED(E_EDIT_DEL_STR),       "Delete current line"},
    {MENU_ITEM,         "Delete &Begin of Line",        K_ED(E_EDIT_DEL_BEGIN),     "Delete begin of line to cursor"},
    {MENU_ITEM,         "Delete &End of Line",          K_ED(E_EDIT_DEL_END),       "Delete end of line from cursor"}
};

static menu_list menuBlock 
{
    {MENU_ITEM,         "&Copy Block",                  K_ED(E_EDIT_BLOCK_COPY),    "Copy selected block"},
    {MENU_ITEM,         "&Move Block",                  K_ED(E_EDIT_BLOCK_MOVE),    "Move selected block"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "Window Cop&y...",              K_APP_WND_COPY,             "Copy block selected in another window"},
    {MENU_ITEM,         "Window Mo&ve...",              K_APP_WND_MOVE,             "Move block selected in another window"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "&Indent In Block",             K_ED(E_EDIT_BLOCK_INDENT),  "Indent inside of selected block"},
    {MENU_ITEM,         "&Unindent In Block",           K_ED(E_EDIT_BLOCK_UNINDENT),"Unindent inside of selected block"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "&Select;Shift+Arrows|Esc,Arrows",K_ED(E_SELECT_BEGIN),     "Begin of stream selection"},
    {MENU_ITEM,         "Select &All",                  K_ED(E_SELECT_ALL),         "Select whole file"},
    {MENU_ITEM,         "Select &Line Mode",            K_ED(E_SELECT_MODE),        "Begin of line selection mode"},
    {MENU_ITEM,         "Select C&olumn Mode",          K_ED(E_SELECT_MODE) | 1,    "Begin of column selection mode"},
    {MENU_ITEM,         "&End Select Mode",             K_ED(E_SELECT_END),         "End of selection mode"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "U&nselect",                    K_ED(E_SELECT_UNSELECT),    "Hide selection"}
};

static menu_list menuSearch 
{
    {MENU_ITEM,         "&Find...",                     K_ED(E_DLG_FIND),           "Find in current window"},
    {MENU_ITEM,         "&Replace...",                  K_ED(E_DLG_REPLACE),        "Find and replace in current window"},
    {MENU_ITEM,         "Repe&at",                      K_ED(E_CTRL_REPEAT),        "Repeat last find/replace operation"},
    {MENU_ITEM,         "Find Again &Up",               K_ED(E_CTRL_FINDUP),        "Repeat last find up"},
    {MENU_ITEM,         "Find Again &Down",             K_ED(E_CTRL_FINDDN),        "Repeat last find down"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "Current Word U&p",             K_ED(E_CTRL_FINDUPW),       "Get word under the cursor and find up"},
    {MENU_ITEM,         "Current Word Dow&n",           K_ED(E_CTRL_FINDDNW),       "Get word under the cursor and find down"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "&Goto Line...",                K_ED(E_DLG_GOTO),           "Go to line number"},
//    {MENU_SEPARATOR},
//    {MENU_ITEM,         "F&ind in Files...",            K_APP_FINDFILE,             "Find in all files"},
//    {MENU_ITEM,         "R&eplace in Files...",         K_APP_REPLACEFILE,          "Find and replace in all files"},
//    {MENU_ITEM,         "Match &List...",               K_APP_FOUNDFILE,            "Get matched files list"}
};

static menu_list menuTools 
{
    {MENU_ITEM,         "Goto &Matched Bracket",        K_ED(E_MOVE_LEX_MATCH),     "Go to opposite bracket"},
//    {MENU_ITEM,         "Fu&nctions List...",           K_ED(E_CTRL_FUNC_LIST),     "Get functions list in file"},
//    {MENU_ITEM,         "Bookmark&s...",                K_APP_BOOKMARK_LIST,        "Set or go to bookmark"},
//    {MENU_ITEM,         "&Compare Files...",            K_APP_DIFF,                 "Compare two files for difference"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "Start/Stop &Record Macro",     K_APP_RECORD_MACRO,         "Start/Stop recording of editing command"},
    {MENU_ITEM,         "&Play Macro",                  K_APP_PLAY_MACRO,           "Repeat recorded editing commands"},
//    {MENU_SEPARATOR},
//    {MENU_ITEM,         "&Options",                     K_MENU + 6,                 "Configuration menu"}
};

static menu_list menuOptions
{
    {MENU_ITEM,         "&Settings...",                 K_APP_SETTINGS,             "Change the application settings"},
    {MENU_ITEM,         "&Colors...",                   K_APP_COLOR,                "Change the application color scheme"}
};

static menu_list menuWindows
{
    {MENU_ITEM,         "&Close Window",                K_ED(E_CTRL_CLOSE),         "Close current window"},
    {MENU_ITEM,         "Close &All Windows",           K_APP_WND_CLOSEALL,         "Close all windows"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "Split/Merge &View",            K_APP_VIEW_SPLIT,           "Split/Merge window view"},
    {MENU_ITEM,         "Split &Horizontal/Vertical",   K_APP_VIEW_MODE,            "Change split type horizontal <-> vertical"},
    {MENU_ITEM,         "&Move Split Line",             K_APP_VIEW_SIZE,            "Change split line position"},
    {MENU_ITEM,         "&Change Active View;Alt+Page", K_APP_VIEW_SET,             "Set another view as active"},
    {MENU_SEPARATOR},
    {MENU_ITEM,         "Windows &List...",             K_APP_WND_LIST,             "Open windows list"}
};

static menu_list menuHelp 
{
//    {MENU_ITEM,         "&View Help",                   K_APP_HELP,               "Help documentation"},
//    {MENU_ITEM,         "Keyboard &Map",                K_APP_HELP_KEYMAP,        "Keyboard command description"},
//    {MENU_SEPARATOR},
    {MENU_ITEM,         "&About...",                    K_APP_ABOUT,                "About Application"}
};

menu_list g_menuRecentFiles 
{
    {MENU_ITEM, "", K_APP_FILE_RECENT}
};

menu_list g_menuRecentSessions
{
    {MENU_ITEM, "", K_APP_SESSION_RECENT},
    {MENU_ITEM, "", K_APP_SESSION_RECENT + 1},
    {MENU_ITEM, "", K_APP_SESSION_RECENT + 2},
    {MENU_ITEM, "", K_APP_SESSION_RECENT + 3},
    {MENU_ITEM, "", K_APP_SESSION_RECENT + 4},
    {MENU_ITEM, "", K_APP_SESSION_RECENT + 5},
    {MENU_ITEM, "", K_APP_SESSION_RECENT + 6},
    {MENU_ITEM, "", K_APP_SESSION_RECENT + 7},
    {MENU_ITEM, "", K_APP_SESSION_RECENT + 8},
    {MENU_ITEM, "", K_APP_SESSION_RECENT + 9}
};

menu_list menuMain
{
    {MENU_ITEM,         "&File",     K_MENU + 1},
    {MENU_ITEM,         "&Edit",     K_MENU + 2},
    {MENU_ITEM,         "&Selection",K_MENU + 3},
    {MENU_ITEM,         "Sea&rch",   K_MENU + 4},
    {MENU_ITEM,         "&Tools",    K_MENU + 5},
    {MENU_ITEM,         "&Windows",  K_MENU + 7},
    {MENU_ITEM,         "&Help",     K_MENU + 8}
};

menu_list g_popupMenu
{
    {MENU_ITEM,       "&Main Menu",         K_MENU,                 "Open main menu"},
    {MENU_ITEM,       "&Window List...",    K_APP_WND_LIST,         "Open windows list"},
    {MENU_ITEM,       "&Properties...",     K_ED(E_CTRL_PROPERTIES),"Get file properties"},
    {MENU_SEPARATOR},
    {MENU_ITEM,       "C&ut",               K_ED(E_EDIT_CB_CUT),    "Cut selected text to clipboard"},
    {MENU_ITEM,       "&Copy",              K_ED(E_EDIT_CB_COPY),   "Copy selected text to clipboard"},
    {MENU_ITEM,       "P&aste",             K_ED(E_EDIT_CB_PASTE),  "Paste text from clipboard"}
};

std::vector<menu_list> g_mainMenu
{
    menuMain,            //0 
    menuFile,            //1
    menuEdit,            //2
    menuBlock,           //3
    menuSearch,          //4
    menuTools,           //5
    menuOptions,         //6
    menuWindows,         //7
    menuHelp,            //8
    g_menuRecentFiles,   //9
    g_menuRecentSessions //10
};

const size_t c_recentFilesMenu{ 9 };
const size_t c_recentSessionsMenu{ 10 };

} //namespace _Editor
