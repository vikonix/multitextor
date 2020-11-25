#include "ColorMap.h"

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
    /*C_WINDOW_FOUND     */           FON_GREEN,
    /*C_WINDOW_DIFF      */                       FON_BLUE | TEXT_RED |                          TEXT_BRIGHT,
    /*C_WINDOW_NOTDIFF   */                       FON_BLUE | TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_WINDOW_CURDIFF   */                       FON_BLUE | TEXT_RED | TEXT_GREEN |             TEXT_BRIGHT,

    /*C_MENU             */ FON_RED | FON_GREEN | FON_BLUE,
    /*C_MENU_BORDER      */ FON_RED | FON_GREEN | FON_BLUE,
    /*C_MENU_B           */ FON_RED | FON_GREEN | FON_BLUE |                         TEXT_BLUE | TEXT_BRIGHT,
    /*C_MENU_SEL         */                                  TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_MENU_B_SEL       */                                  TEXT_RED | TEXT_GREEN | TEXT_BLUE | TEXT_BRIGHT,
    /*C_MENU_DISABLED    */ FON_RED | FON_GREEN | FON_BLUE |            TEXT_GREEN | TEXT_BLUE,

    /*C_DIALOG           */ FON_RED | FON_GREEN | FON_BLUE,
    /*C_DIALOG_BORDER    */ FON_RED | FON_GREEN | FON_BLUE,
    /*C_DIALOG_TITLE     */ FON_RED | FON_GREEN | FON_BLUE |                         TEXT_BLUE,
    /*C_DIALOG_INFO      */ FON_RED | FON_GREEN | FON_BLUE |                         TEXT_BLUE | TEXT_BRIGHT,
    /*C_DIALOG_SEL       */                       FON_BLUE | TEXT_RED | TEXT_GREEN             | TEXT_BRIGHT,
    /*C_DIALOG_DISABLED  */ FON_RED | FON_GREEN | FON_BLUE |            TEXT_GREEN | TEXT_BLUE,
    /*C_DIALOG_FIELD     */           FON_GREEN | FON_BLUE | TEXT_RED | TEXT_GREEN | TEXT_BLUE | TEXT_BRIGHT,
    /*C_DIALOG_FIELD_SEL */                       FON_BLUE | TEXT_RED | TEXT_GREEN | TEXT_BLUE,
    /*C_DIALOG_FIELD_ACT */           FON_GREEN | FON_BLUE |            TEXT_GREEN | TEXT_BLUE | TEXT_BRIGHT,
    /*C_SHADE            */                       FON_BLUE,
};

