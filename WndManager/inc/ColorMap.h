#pragma once

#include "Types.h"
#include "Color.h"


//////////////////////////////////////////////////////////////////////////////
enum 
{
  C_SCREEN = 0,
  C_VIEW_SPLITTER,
  C_ACCESS_MENU,
  C_ACCESS_MENU_B,
  C_STATUS_LINE,
  C_STATUS_LINE_G,
  C_STATUS_LINE_B,
  C_CLOCK,
  C_WINDOW,
  C_WINDOW_TAB,
  C_WINDOW_LEX_REM,
  C_WINDOW_LEX_CONST,
  C_WINDOW_LEX_KEYW,
  C_WINDOW_LEX_DELIM,
  C_WINDOW_LEX_MATCH,
  C_WINDOW_BORDER,
  C_WINDOW_TITLE,
  C_WINDOW_INFO,
  C_WINDOW_SEL,
  C_WINDOW_FOUND,
  C_WINDOW_DIFF,
  C_WINDOW_NOTDIFF,
  C_WINDOW_CURDIFF,
  C_MENU,
  C_MENU_BORDER,
  C_MENU_B,
  C_MENU_SEL,
  C_MENU_B_SEL,
  C_MENU_DISABLED,
  C_DIALOG,
  C_DIALOG_BORDER,
  C_DIALOG_TITLE,
  C_DIALOG_INFO,
  C_DIALOG_SEL,
  C_DIALOG_DISABLED,
  C_DIALOG_FIELD,
  C_DIALOG_FIELD_SEL,
  C_DIALOG_FIELD_ACT,
  C_SHADE,

  C_COUNT
};


//////////////////////////////////////////////////////////////////////////////
extern color_t g_ColorMap[];

#define ColorScreen         (g_ColorMap[C_SCREEN])
#define ColorViewSplitter   (g_ColorMap[C_VIEW_SPLITTER])
#define ColorAccessMenu     (g_ColorMap[C_ACCESS_MENU])
#define ColorAccessMenuB    (g_ColorMap[C_ACCESS_MENU_B])
#define ColorStatusLine     (g_ColorMap[C_STATUS_LINE])
#define ColorStatusLineG    (g_ColorMap[C_STATUS_LINE_G])
#define ColorStatusLineB    (g_ColorMap[C_STATUS_LINE_B])
#define ColorClock          (g_ColorMap[C_CLOCK])
#define ColorWindow         (g_ColorMap[C_WINDOW])
#define ColorWindowTab      (g_ColorMap[C_WINDOW_TAB])
#define ColorWindowLRem     (g_ColorMap[C_WINDOW_LEX_REM])
#define ColorWindowLConst   (g_ColorMap[C_WINDOW_LEX_CONST])
#define ColorWindowLKeyW    (g_ColorMap[C_WINDOW_LEX_KEYW])
#define ColorWindowLDelim   (g_ColorMap[C_WINDOW_LEX_DELIM])
#define ColorWindowLMatch   (g_ColorMap[C_WINDOW_LEX_MATCH])
#define ColorWindowBorder   (g_ColorMap[C_WINDOW_BORDER])
#define ColorWindowTitle    (g_ColorMap[C_WINDOW_TITLE])
#define ColorWindowInfo     (g_ColorMap[C_WINDOW_INFO])
#define ColorWindowSelect   (g_ColorMap[C_WINDOW_SEL])
#define ColorWindowFound    (g_ColorMap[C_WINDOW_FOUND])
#define ColorWindowDiff     (g_ColorMap[C_WINDOW_DIFF])
#define ColorWindowNotDiff  (g_ColorMap[C_WINDOW_NOTDIFF])
#define ColorWindowCurDiff  (g_ColorMap[C_WINDOW_CURDIFF])
#define ColorMenu           (g_ColorMap[C_MENU])
#define ColorMenuBorder     (g_ColorMap[C_MENU_BORDER])
#define ColorMenuB          (g_ColorMap[C_MENU_B])
#define ColorMenuSel        (g_ColorMap[C_MENU_SEL])
#define ColorMenuBSel       (g_ColorMap[C_MENU_B_SEL])
#define ColorMenuDisabled   (g_ColorMap[C_MENU_DISABLED])
#define ColorDialog         (g_ColorMap[C_DIALOG])
#define ColorDialogBorder   (g_ColorMap[C_DIALOG_BORDER])
#define ColorDialogTitle    (g_ColorMap[C_DIALOG_TITLE])
#define ColorDialogInfo     (g_ColorMap[C_DIALOG_INFO])
#define ColorDialogSelect   (g_ColorMap[C_DIALOG_SEL])
#define ColorDialogDisabled (g_ColorMap[C_DIALOG_DISABLED])
#define ColorDialogField    (g_ColorMap[C_DIALOG_FIELD])
#define ColorDialogFieldSel (g_ColorMap[C_DIALOG_FIELD_SEL])
#define ColorDialogFieldAct (g_ColorMap[C_DIALOG_FIELD_ACT])
#define ColorShade          (g_ColorMap[C_SHADE])

