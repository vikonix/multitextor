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
#include "Dialogs/EditorDialogs.h"
#include "DlgControls.h"
#include "App.h"


/////////////////////////////////////////////////////////////////////////////
#define ID_DP_PATH         (ID_USER +  1)
#define ID_DP_NAME         (ID_USER +  2)
#define ID_DP_INFO         (ID_USER +  3)
#define ID_DP_COLOR        (ID_USER +  4)
#define ID_DP_CP           (ID_USER +  5)
#define ID_DP_CRLF         (ID_USER +  6)
#define ID_DP_TAB          (ID_USER +  7)
#define ID_DP_TAB_CONVERT  (ID_USER +  8)
#define ID_DP_TAB_SAVE     (ID_USER +  9)
#define ID_DP_TAB_SHOW     (ID_USER + 10)
#define ID_DP_LOG          (ID_USER + 11)
#define ID_DP_RO           (ID_USER + 12)

PropertiesVars PropertiesDialog::s_vars;

std::list<control> propertiesDialog
{
    {CTRL_TITLE,                        "File Properties",      0,                  nullptr,                          1,  0, 70, 17},

    {CTRL_STATIC,                       "",                     ID_DP_PATH,         nullptr,                          1,  1, 66,  7},
    {CTRL_STATIC,                       "",                     ID_DP_NAME,         nullptr,                          1,  2, 66,  7},
    {CTRL_STATIC,                       "",                     ID_DP_INFO,         nullptr,                          1,  3, 66,  7},
    {CTRL_LINE,                         "",                     0,                  nullptr,                          1,  5, 66},

    {CTRL_STATIC,                       "&File type:",          0,                  nullptr,                          1,  6, 14},
    {CTRL_DROPLIST,                     "",                     ID_DP_COLOR,        &PropertiesDialog::s_vars.type,  15,  6, 17,  7, "Select file type"},
    {CTRL_STATIC,                       "Code &page:",          0,                  nullptr,                          1,  7, 14},
    {CTRL_DROPLIST,                     "",                     ID_DP_CP,           &PropertiesDialog::s_vars.cp,    15,  7, 17,  7, "Select encoding page"},

    {CTRL_CHECK,                        "&Read only",           ID_DP_RO,           &PropertiesDialog::s_vars.ro,     1,  9, 30,  1, "Protect file from changing"},
    {CTRL_CHECK,                        "&Log file",            ID_DP_LOG,          &PropertiesDialog::s_vars.log,    1, 10, 30,  1, "File will be reload without confirmation if it changed"},

    {CTRL_STATIC,                       "&End of line:",        0,                  nullptr,                          35,  6, 13},
    {CTRL_DROPLIST,                     "",                     ID_DP_CRLF,         &PropertiesDialog::s_vars.eol,    50,  6, 17,  6, "Select the end of line type"},

    {CTRL_STATIC,                       "&Tab size:",           0,                  nullptr,                          35,  8, 13},
    {CTRL_EDIT,                         "",                     ID_DP_TAB,          &PropertiesDialog::s_vars.tabs,   65,  8,  2,  7, "Input tabulation size (1-10)"},
    {CTRL_RADIO,                        "Convert tabs to &space",ID_DP_TAB_CONVERT, &PropertiesDialog::s_vars.saveTab,35,  9, 30,  1, "Convert all tabulations to space"},
    {CTRL_RADIO,                        "&Use tabs as space",   ID_DP_TAB_SAVE,     &PropertiesDialog::s_vars.saveTab,35, 10, 30,  1, "Save tabulations"},
    {CTRL_CHECK,                        "S&how tabs",           ID_DP_TAB_SHOW,     &PropertiesDialog::s_vars.showTab,35, 11, 30,  1, "Highlight tabulations"},

    {CTRL_LINE,                         "",                     0,                  nullptr,                            1, 13, 66},
    {CTRL_DEFBUTTON | CTRL_ALIGN_RIGHT, "Ok",                   ID_OK,              nullptr,                           50, 14,  0,  0, "Apply changes and reload file if need"},
    {CTRL_BUTTON | CTRL_ALIGN_RIGHT,    "Cancel",               ID_CANCEL,          nullptr,                           60, 14}
};

PropertiesDialog::PropertiesDialog(pos_t x, pos_t y)
    : Dialog(propertiesDialog, x, y)
{
}

bool PropertiesDialog::OnActivate()
{
    return true;
}

bool PropertiesDialog::OnClose(int id)
{
    return true;
}

/*
int PropertiesDialog::OnActivate()
{
    CtrlStatic* pPath = (CtrlStatic*)GetItem(ID_DP_PATH);

    short x, y, sizex, sizey;
    pPath->GetPos(&x, &y, &sizex, &sizey);

    char* pFile = g_WndProp.path;
    char buff[MAX_PATH + 1];

    if (strlen(pFile) <= sizex)
    {
        pPath->SetName(pFile);
    }
    else
    {
        char* pName = SDir::FindLastName(pFile);
        char c = *pName;
        *pName = 0;

        SDir::CutPath(pFile, buff, sizex);
        pPath->SetName(buff);

        pPath = (CtrlStatic*)GetItem(ID_DP_NAME);
        *pName = c;
        SDir::CutPath(pName, buff, sizex);
        pPath->SetName(buff);
    }

    CtrlStatic* pInfo = (CtrlStatic*)GetItem(ID_DP_INFO);


    char ts[128];
    int rc = ctime_s(ts, sizeof(ts), &g_WndProp.time);
    assert(rc == 0);
    (void)rc;

    char* pTime = ts;
    while (*pTime != ' ')
        //skip week day
        ++pTime;
    while (*pTime == ' ')
        //skip week day
        ++pTime;
    //del EOL
    pTime[strlen(pTime) - 1] = 0;

    sprintf_s(buff, sizeof(buff), GetSStr(STR_D(PRDS_Bytes)), g_WndProp.size, pTime);
    pInfo->SetName(buff);

    CtrlSList* pCtrl = (CtrlSList*)GetItem(ID_DP_COLOR);
    size_t jeton = 0;
    const char* pLex;
    while ((pLex = g_LexCfg.Enum(&jeton)) != NULL)
        pCtrl->AppendStr((char*)pLex);
    pCtrl->SetSelect(g_LexCfg.GetCfgN(g_WndProp.parsemode));

    pCtrl = (CtrlSList*)GetItem(ID_DP_CRLF);
    pCtrl->AppendStr("UNIX  (LF)");
    pCtrl->AppendStr("DOS   (CR+LF)");
    pCtrl->AppendStr("MAC   (CR)");
    pCtrl->SetSelect(g_WndProp.crlf);

    pCtrl = (CtrlSList*)GetItem(ID_DP_CP);
    for (int i = 0; ; ++i)
    {
        const char* pCP = EnumCPname(i);
        if (!pCP)
            break;
        pCtrl->AppendStr(pCP);
    }
    pCtrl->SetSelect(GetCPindex(g_WndProp.cp));

    CtrlEdit* pTab = (CtrlEdit*)GetItem(ID_DP_TAB);
    sprintf_s(buff, sizeof(buff), "%d", g_WndProp.tabsize);
    pTab->SetName(buff);

    return 0;
}


int PropertiesDialog::OnClose(int id)
{
    if (id == ID_OK)
    {
        char buff[15] = { 0 };
        int tab = 0;

        CtrlEdit* pEdit = (CtrlEdit*)GetItem(ID_DP_TAB);
        if (pEdit && pEdit->GetName(buff, sizeof(buff))
            && ScanDec(buff, 1, 10, &tab))
        {
            TPRINT(("err buff=%s tab=%d\n", buff, tab));
            SetErrorLine(STR_D(PRDE_BadTabSize));
            SelectItem(ID_DP_TAB);
            Refresh();
            return -1;
        }

        g_WndProp.tabsize = tab;

        CtrlSList* pCtrl = (CtrlSList*)GetItem(ID_DP_CP);
        g_WndProp.cp = EnumCP(pCtrl->GetSelect());

        pCtrl = (CtrlSList*)GetItem(ID_DP_COLOR);
        g_WndProp.parsemode = g_LexCfg.GetCfgName(pCtrl->GetSelect());

        pCtrl = (CtrlSList*)GetItem(ID_DP_CRLF);
        g_WndProp.crlf = pCtrl->GetSelect();
    }

    return 0;
}
*/