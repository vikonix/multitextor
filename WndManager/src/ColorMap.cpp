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
#include "WndManager/ColorMap.h"

namespace _WndManager
{

color_t g_ColorMap[C_COUNT] 
{
    /*C_SCREEN           */                                  TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_VIEW_SPLITTER    */                                  TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_ACCESS_MENU      */           FON_GREEN | FON_BLUE,
    /*C_ACCESS_MENU_B    */                                  TEXT_RED | TEXT_GREEN | TEXT_BLUE,

    /*C_STATUS_LINE      */ FON_RED | FON_GREEN | FON_BLUE,
    /*C_STATUS_LINE_G    */ FON_RED | FON_GREEN | FON_BLUE |            TEXT_GREEN | TEXT_BLUE,
    /*C_STATUS_LINE_B    */ FON_RED |                        TEXT_RED | TEXT_GREEN | TEXT_BLUE | TEXT_BRIGHT,
    /*C_CLOCK            */ FON_RED | FON_GREEN | FON_BLUE,

    /*C_WINDOW           */                       FON_BLUE |            TEXT_GREEN | TEXT_BLUE | TEXT_BRIGHT,
    /*C_WINDOW_TAB       */                                  TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_WINDOW_LEX_REM   */                       FON_BLUE | TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_WINDOW_LEX_CONST */                       FON_BLUE |            TEXT_GREEN |             TEXT_BRIGHT,
    /*C_WINDOW_LEX_KEYW  */                       FON_BLUE | TEXT_RED | TEXT_GREEN | TEXT_BLUE | TEXT_BRIGHT,
    /*C_WINDOW_LEX_DELIM */                       FON_BLUE | TEXT_RED | TEXT_GREEN |             TEXT_BRIGHT,
    /*C_WINDOW_LEX_MATCH */                       FON_BLUE | TEXT_RED |                          TEXT_BRIGHT,

    /*C_WINDOW_BORDER    */                       FON_BLUE | TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_WINDOW_TITLE     */           FON_GREEN | FON_BLUE,
    /*C_WINDOW_INFO      */           FON_GREEN | FON_BLUE | TEXT_RED | TEXT_GREEN | TEXT_BLUE | TEXT_BRIGHT,
    /*C_WINDOW_SEL       */ FON_RED | FON_GREEN | FON_BLUE,
    /*C_WINDOW_SEL_LMATCH*/ FON_RED | FON_GREEN | FON_BLUE | TEXT_RED |                          TEXT_BRIGHT,
    /*C_WINDOW_FOUND     */           FON_GREEN,
    /*C_WINDOW_DIFF      */                       FON_BLUE | TEXT_RED |                          TEXT_BRIGHT,
    /*C_WINDOW_NOTDIFF   */                       FON_BLUE | TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_WINDOW_CURDIFF   */                       FON_BLUE | TEXT_RED | TEXT_GREEN |             TEXT_BRIGHT,

    /*C_MENU             */ FON_RED | FON_GREEN | FON_BLUE,
    /*C_MENU_BORDER      */ FON_RED | FON_GREEN | FON_BLUE,
    /*C_MENU_B           */ FON_RED | FON_GREEN | FON_BLUE |                         TEXT_BLUE | TEXT_BRIGHT,
    /*C_MENU_DISABLED    */ FON_RED | FON_GREEN | FON_BLUE |            TEXT_GREEN | TEXT_BLUE,
    /*C_MENU_SEL         */                                  TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_MENU_B_SEL       */                                  TEXT_RED | TEXT_GREEN | TEXT_BLUE | TEXT_BRIGHT,

    /*C_DIALOG           */ FON_RED | FON_GREEN | FON_BLUE,
    /*C_DIALOG_BORDER    */ FON_RED | FON_GREEN | FON_BLUE,
    /*C_DIALOG_TITLE     */ FON_RED | FON_GREEN | FON_BLUE |                         TEXT_BLUE,
    /*C_DIALOG_INFO      */ FON_RED | FON_GREEN | FON_BLUE |                         TEXT_BLUE | TEXT_BRIGHT,
    /*C_DIALOG_DISABLED  */ FON_RED | FON_GREEN | FON_BLUE |            TEXT_GREEN | TEXT_BLUE,
    /*C_DIALOG_SEL       */                       FON_BLUE | TEXT_RED | TEXT_GREEN             | TEXT_BRIGHT,
    /*C_DIALOG_FIELD_SEL */                       FON_BLUE | TEXT_RED | TEXT_GREEN | TEXT_BLUE | TEXT_BRIGHT,
    /*C_DIALOG_FIELD     */           FON_GREEN | FON_BLUE,// | TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_DIALOG_FIELD_ACT */           FON_GREEN | FON_BLUE | TEXT_RED | TEXT_GREEN | TEXT_BLUE | TEXT_BRIGHT,
    /*C_SHADE            */                       FON_BLUE,
};

} //namespace _WndManager 
