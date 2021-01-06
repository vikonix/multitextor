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
#include "Dialogs/StdDialogs.h"
#include "DlgControls.h"
#include "WndManager.h"

/////////////////////////////////////////////////////////////////////////////
#define ID_OF_NAME      (ID_USER + 1)
#define ID_OF_PATH      (ID_USER + 2)
#define ID_OF_FILELIST  (ID_USER + 3)
#define ID_OF_DIRLIST   (ID_USER + 4)
#define ID_OF_STAT_CP   (ID_USER + 5)
#define ID_OF_CP        (ID_USER + 6)
#define ID_OF_STAT_TYPE (ID_USER + 7)
#define ID_OF_TYPE      (ID_USER + 8)
#define ID_OF_RO        (ID_USER + 9)
#define ID_OF_LOG       (ID_USER + 10)
#define ID_OF_INFO      (ID_USER + 11)

struct FileDialogVars
{
    std::string mask{"*.*"};
    int  type{};
    int  cp{};
    bool ro{};
    bool log{};
};

FileDialogVars fdVars;

std::list<control> fileDialog {
  {CTRL_TITLE,                      "",             0,              nullptr,         1,  0, 70, 21},

  {CTRL_STATIC,                     "File &name:",  0,              nullptr,         1,  1, 14},
  {CTRL_EDITDROPLIST,               "",             ID_OF_NAME,     &fdVars.mask,   15,  1, 52,  7, "Input file name or mask"},

  {CTRL_STATIC | CTRL_NOCOLOR,      "",             ID_OF_PATH,     nullptr,         1,  3, 66},

  {CTRL_LIST,                       "&Directories", ID_OF_DIRLIST,  (int*)nullptr,   0,  4, 19, 14, "Select directory"},
  {CTRL_LIST,                       "&Files",       ID_OF_FILELIST, (int*)nullptr,  19,  4, 34, 14, "Select file name"},

  {CTRL_DEFBUTTON | CTRL_ALIGN_LEFT,"",             ID_OK,          nullptr,        54,  5},
  {CTRL_BUTTON | CTRL_ALIGN_LEFT,   "Cancel",       ID_CANCEL,      nullptr,        54,  7},

  {CTRL_STATIC,                     "File &type:",  ID_OF_STAT_TYPE,nullptr,        54,  9, 14},
  {CTRL_DROPLIST,                   "",             ID_OF_TYPE,     &fdVars.type,   54, 10, 13,  6, "Select file type"},
  {CTRL_STATIC,                     "Code &page:",  ID_OF_STAT_CP,  nullptr,        54, 12, 14},
  {CTRL_DROPLIST,                   "",             ID_OF_CP,       &fdVars.cp,     54, 13, 13,  6, "Select file code page"},

  {CTRL_CHECK,                      "&Read only",   ID_OF_RO,       &fdVars.ro,     54, 15,  0,  0, "Open file as read only"},
  {CTRL_CHECK,                      "&Log file",    ID_OF_LOG,      &fdVars.log,    54, 16,  0,  0, "Opne file that can grow"},
  {CTRL_STATIC,                     "",             ID_OF_INFO,     nullptr,        20, 18, 34},
  {CTRL_LINE,                       "",             0,              nullptr,        54, 17, 13}
};

FileDialog::FileDialog(FileDlgMode mode, pos_t x, pos_t y)
    : Dialog(fileDialog, x, y)
    , m_mode{ mode }
{
}

bool FileDialog::OnActivate()
{
    bool hide{false};

    switch (m_mode)
    {
    case FileDlgMode::Open:
        GetItem(0)->SetName("Open File");
        GetItem(ID_OK)->SetName("Open");
        GetItem(ID_OK)->SetHelpLine("Open file in new place");
        break;
    case FileDlgMode::Load:
        GetItem(0)->SetName("Load File");
        GetItem(ID_OK)->SetName("Load");
        GetItem(ID_OK)->SetHelpLine("Load file in same place");
        break;
    case FileDlgMode::Save:
        GetItem(0)->SetName("Save File As");
        GetItem(ID_OK)->SetName("Save");
        GetItem(ID_OK)->SetHelpLine("Save file with new name");
        hide = true;
        break;
    }

    if (hide)
    {
        GetItem(ID_OF_STAT_TYPE)->SetMode(CTRL_HIDE);
        GetItem(ID_OF_TYPE)->SetMode(CTRL_HIDE);
        GetItem(ID_OF_STAT_CP)->SetMode(CTRL_HIDE);
        GetItem(ID_OF_CP)->SetMode(CTRL_HIDE);
        GetItem(ID_OF_RO)->SetMode(CTRL_HIDE);
        GetItem(ID_OF_LOG)->SetMode(CTRL_HIDE);
    }

    auto cp = GetItem(ID_OF_CP);
    auto ctrlCp = std::dynamic_pointer_cast<CtrlDropList>(cp);
    if (ctrlCp)
    {
        for (const std::string& str : { "437", "866", "1251" })
            ctrlCp->AppendStr(str);
        ctrlCp->SetSelect(fdVars.cp);
    }

    auto type = GetItem(ID_OF_TYPE);
    auto ctrlType = std::dynamic_pointer_cast<CtrlDropList>(type);
    if (ctrlType)
    {
        for (const std::string& str : { "Text", "C++" })
            ctrlType->AppendStr(str);
        ctrlType->SetSelect(fdVars.type);
    }

    /*
    pCtrl = (CtrlSList*)GetItem(ID_OF_TYPE);
    size_t jeton = 0;
    const char* pLex;
    while ((pLex = g_LexCfg.Enum(&jeton)) != NULL)
        pCtrl->AppendStr((char*)pLex);

    char buff[MAX_PATH + 1];
    char* pMask = SaveFileM.GetStr(0);
    if (!pMask)
        pMask = "*.*";

    if (m_nMode == DIALOG_NEWSESS || m_nMode == DIALOG_OPENSESS)
        pMask = SESSION_MASK;
    else if (!strcmp(pMask, SESSION_MASK))
    {
        SaveFileM.DelStr(0);
        pMask = SaveFileM.GetStr(0);
        if (!pMask)
            pMask = "*.*";
    }

    SDir::AppendName(sFilePath, pMask, buff);

    TPRINT(("OnActivate %s\n", buff));
    m_DList.SetFullPath(buff);

    //ReadDir();
*/
    return true;
}

#if 0
int FileDialog::DialogProc(int code)
{
    //TPRINT(("code=%x\n", code));

    switch (code >> 16)
    {
    case K_SELECT >> 16:
        if (GetSelectedId() == ID_OF_FILELIST)
        {
            //file list selected
            int item = code & K_CODEMASK;
            FileInfo* pInfo = m_DList.GetFileInfo(item);
            if (!pInfo)
                break;

            char ts[128];
            int rc = ctime_s(ts, sizeof(ts), &pInfo->time);
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

            char buff[128];
            if (pInfo->nSize > 1024 * 1024 * 1024)
            {
                int s = (int)(pInfo->nSize / (1024 * 1024));
                sprintf_s(buff, sizeof(buff), "%s  %9dM", pTime, s);
            }
            else if (pInfo->nSize >= 1024 * 1024)
            {
                int s = (int)(pInfo->nSize / 1024);
                sprintf_s(buff, sizeof(buff), "%s  %9dK", pTime, s);
            }
            else
            {
                int s = (int)pInfo->nSize;
                sprintf_s(buff, sizeof(buff), "%s  %10d", pTime, s);
            }

            CtrlStatic* pFInfo = (CtrlStatic*)GetItem(ID_OF_INFO);
            pFInfo->SetName(buff);

            if (m_nMode != DIALOG_NEWSESS && m_nMode != DIALOG_SAVEAS)
            {
                CtrlCombo* pCombo = (CtrlCombo*)GetItem(ID_OF_NAME);
                pCombo->SetName(pInfo->sName);

                if (m_nMode != DIALOG_OPENSESS && m_nMode != DIALOG_CMD)// && m_nMode != DIALOG_KEY)
                {
                    CtrlSList* pCtrl = (CtrlSList*)GetItem(ID_OF_TYPE);
                    pCtrl->SetSelect(g_LexCfg.CheckFile(pInfo->sName));
                }
            }
        }
        else if (GetSelectedId() == ID_OF_DIRLIST)
        {
            //file list selected
            int item = code & K_CODEMASK;
            FileInfo* pInfo = m_DList.GetDirInfo(item);
            if (!pInfo)
                break;

            if (m_nMode != DIALOG_NEWSESS && m_nMode != DIALOG_SAVEAS)
            {
                CtrlCombo* pCombo = (CtrlCombo*)GetItem(ID_OF_NAME);
                pCombo->SetName(pInfo->sName);
            }
        }
        break;

    case K_SYMBOL >> 16:
        if (code == K_ENTER)
        {
            if (GetSelectedId() == ID_OF_DIRLIST)
            {
                //directory list selected
                CtrlList* pDList = (CtrlList*)GetItem(ID_OF_DIRLIST);
                int item = pDList->GetSelect();
                size_t l;
                char* pDir = pDList->GetSelectedStr(item, &l);
                char buff[MAX_PATH + 1];
                if (pDir)
                {
                    memcpy(buff, pDir, l);
                    buff[l] = 0;
                }
                else
                    buff[0] = 0;

                m_DList.SetFullPath(buff);
                ReadDir();
                code = 0;
            }
            else if (GetSelectedId() == ID_OF_FILELIST)
            {
                //file list enter
                CtrlList* pDList = (CtrlList*)GetItem(ID_OF_FILELIST);
                int item = pDList->GetSelect();
                FileInfo* pInfo = m_DList.GetFileInfo(item);
                if (!pInfo)
                    break;

                CtrlCombo* pCombo = (CtrlCombo*)GetItem(ID_OF_NAME);
                pCombo->SetName(pInfo->sName);
            }
            else
            {
                char buff[MAX_PATH + 1];
                CtrlCombo* pCombo = (CtrlCombo*)GetItem(ID_OF_NAME);
                pCombo->GetName(buff, sizeof(buff));

                size_t m = m_DList.SetFullPath(buff);

                //сохраняем тип разбора
                CtrlSList* pCtrl = (CtrlSList*)GetItem(ID_OF_TYPE);
                int parse = pCtrl->GetSelect();

                int n = ReadDir();
                if (!m || (n != 0 && n != 1))
                {
                    //if not simple mask or found many files
                    code = 0;
                }
                else
                {
                    //восстанавливаем тип разбора
                    pCtrl->SetSelect(parse);
                }
            }
        }
        else if (code == K_BS)
        {
            if (GetSelectedId() == ID_OF_DIRLIST)
            {
                //TPRINT(("BS\n"));
                m_DList.SetFullPath("..");
                ReadDir();
                code = 0;
            }
        }
        break;
    }

    return code;
}


int FileDialog::ReadDir()
{
    m_DList.ReadDir();

    CtrlCombo* pCombo = (CtrlCombo*)GetItem(ID_OF_NAME);
    if (m_nMode != DIALOG_NEWSESS && m_nMode != DIALOG_SAVEAS)
        pCombo->SetName("");
    CtrlStatic* pFInfo = (CtrlStatic*)GetItem(ID_OF_INFO);
    pFInfo->SetName("");
    CtrlStatic* pDir = (CtrlStatic*)GetItem(ID_OF_PATH);

    char buff[MAX_PATH + 1];
    SDir::AppendName(m_DList.GetPath(), m_DList.GetMask(), buff);

    char buff1[MAX_PATH + 1];
    short x, y, sizex, sizey;
    pDir->GetPos(&x, &y, &sizex, &sizey);
    SDir::CutPath(buff, buff1, sizex);
    pDir->SetName(buff1);

    CtrlList* pFList = (CtrlList*)GetItem(ID_OF_FILELIST);
    pFList->Clear();
    int n;
    for (n = 0; n < MAX_FILES_NUM; ++n)
    {
        FileInfo* pInfo = m_DList.GetFileInfo(n);
        if (!pInfo)
            break;

        int rc = pFList->AppendStr(pInfo->sName);
        if (rc <= 0)
            break;
    }
    TPRINT(("FileList %d\n", n));

    CtrlList* pDList = (CtrlList*)GetItem(ID_OF_DIRLIST);
    pDList->Clear();

    int i;
    for (i = 0; i < MAX_FILES_NUM; ++i)
    {
        FileInfo* pInfo = m_DList.GetDirInfo(i);
        if (!pInfo)
            break;

        pDList->AppendStr(pInfo->sName);
    }

    for (int j = 0; j + i < MAX_FILES_NUM; ++j)
    {
        FileInfo* pInfo = m_DList.GetDrvInfo(j);
        if (!pInfo)
            break;

        pDList->AppendStr(pInfo->sName);
    }

    int item = SelectItem(ID_OF_FILELIST);
    DialogProc(K_SELECT);

    SelectItem(item);
    Refresh();

    return n;
}


int FileDialog::OnClose(int id)
{
    if (id == ID_OK)
    {
        char buff[MAX_PATH + 1];
        CtrlCombo* pCombo = (CtrlCombo*)GetItem(ID_OF_NAME);
        pCombo->GetName(buff, sizeof(buff));
        pCombo->SetName(m_DList.GetMMask());

        strcpy_s(sFilePath, sizeof(sFilePath), m_DList.GetPath());
        strcpy_s(g_InputPath, sizeof(g_InputPath), sFilePath);

        if (m_nMode != DIALOG_CMD)
        {
            if (buff[0])
                //найденный файл
                SDir::AppendName(sFilePath, buff, g_InputStr);
            else
                //новый файл
                SDir::AppendName(sFilePath, m_DList.GetMask(), g_InputStr);
        }
        else
        {
            strcpy_s(g_InputStr, sizeof(g_InputStr), "." F_SLASH);
            //strcpy(g_InputStr, "");
            if (buff[0])
                //найденный файл
                strcat_s(g_InputStr, sizeof(g_InputStr), buff);
            else
                //новый файл
                strcat_s(g_InputStr, sizeof(g_InputStr), m_DList.GetMask());
        }

        if (strchr(g_InputStr, '*') || strchr(g_InputStr, '?'))
        {
            TPRINT(("ERROR empty name %s\n", g_InputStr));
            return -1;
        }

        if (!FileObject::IsFile(g_InputStr))
        {
            TPRINT(("ERROR directory name %s\n", g_InputStr));
            return -1;
        }

        CtrlSList* pCtrl = (CtrlSList*)GetItem(ID_OF_CP);
        TextBuff::s_fileCP = EnumCP(pCtrl->GetSelect());

        pCtrl = (CtrlSList*)GetItem(ID_OF_TYPE);
        g_pParseName = g_LexCfg.GetCfgName(pCtrl->GetSelect());

        TPRINT(("FileDialog::OnClose %s cp=%d\n", g_InputStr, TextBuff::s_fileCP));
    }

    return 0;
}
#endif

#if 0
#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif
#include <string.h>

#include "keycode.h"
#include "app.h"
#include "diff.h"
#include "mdialog.h"
#include "scan.h"
#include "info.h"
#include "mstring.h"

#include "debug.h"


int CheckOpenFile(char* pName);


/////////////////////////////////////////////////////////////////////////////
//extern int  g_fileCP;

char g_InputStr[MAX_PATH + 1];
char g_InputPath[MAX_PATH + 1];

const char* g_pParseName  = "";
int g_fRO                 = 0;
int g_fLog                = 0;
int g_fMarkDiff           = 0;

/////////////////////////////////////////////////////////////////////////////
//variables for save
static char sFilePath[MAX_PATH + 1] = {0};
static char sFindPath[MAX_PATH + 1] = {0};

static StrSaveList SaveFind;
static StrSaveList SaveReplace;
static StrSaveList SaveFileM;
static StrSaveList SaveFindM;


static int iRecurs = 0;
static int iInOpen = 0;

static const char* pFileCP;

DVar DlgParam[] = {
  {"sfind",     VAR_LSTR, &SaveFind},
  {"sreplace",  VAR_LSTR, &SaveReplace},
  {"sfilem",    VAR_LSTR, &SaveFileM},
  {"sfindm",    VAR_LSTR, &SaveFindM},

  {"filepath",  VAR_BSTR, &sFilePath},
  {"findpath",  VAR_BSTR, &sFindPath},
  {"filecp",    VAR_STR,  &pFileCP},
  {"frecurs",   VAR_INT,  &iRecurs},
  {"finopen",   VAR_INT,  &iInOpen},

  {"fcases",    VAR_INT,  &g_fCaseSensitive},
  {"fwword",    VAR_INT,  &g_fWholeWord},

  {"mdiff",     VAR_INT,  &g_fMarkDiff},
  {0}
};


void PrepareToLoad(int fAll)
{
  if(fAll)
  {
    SaveFind.Clear();
    SaveReplace.Clear();
    SaveFileM.Clear();
    SaveFindM.Clear();
  }
  sFilePath[0] = 0;
  sFindPath[0] = 0;
}


int DialogSection(int mode)
{
  (void) mode;
  TextBuff::s_fileCP = GetCP(pFileCP);
  return 0;
}


void PrepareToSave()
{
  char Buff[MAX_PATH + 1];
  SDir::GetRelPath(sFilePath, Buff, sizeof(Buff));
  strcpy_s(sFilePath, sizeof(sFilePath), Buff);
  SDir::GetRelPath(sFindPath, Buff, sizeof(Buff));
  strcpy_s(sFindPath, sizeof(sFindPath), Buff);
  pFileCP = GetCPname(TextBuff::s_fileCP);
}


int SaveFindStr(char* pStr)
{
  SaveFind.AddStr(pStr);
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
#define ID_AD_SITE  (ID_USER + 1)
#define ID_AD_USER  (ID_USER + 2)
#define ID_AD_PR_ID (ID_USER + 3)
#define ID_AD_INST  (ID_USER + 4)

control dlgAbout[] = {
  {CTRL_TITLE,                      STR_D(AD_AboutCaption),               0, NULL,  1,  0, 45, 11},

  {CTRL_STATIC,                     STR_D(AD_Version),                    0, NULL,  1,  1, 51},
  {CTRL_STATIC,                     STR_D(AD_Description),                0, NULL,  1,  2, 41},
  {CTRL_STATIC,                     STR_D(AD_Site),              ID_AD_SITE, NULL,  1,  4, 41},
  {CTRL_STATIC,                     STR_D(AD_Copyright),                  0, NULL,  1,  5, 41},
  {CTRL_LINE,                       "",                                   0, NULL,  1,  7, 41},

  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, STR_D(AD_KeyOK),                  ID_OK, NULL, 29, 8},
  {0}
};


/////////////////////////////////////////////////////////////////////////////
control dlgExit[] = {
  {CTRL_TITLE,                   STR_D(ED_ExitCaption),      0, NULL,  1, 0, 35, 7},

  {CTRL_STATIC,                  STR_D(ED_DoYouWantToExit),  0, NULL,  1, 1, 31},
  {CTRL_LINE,                    "",                         0, NULL,  1, 3, 31},

  {CTRL_BUTTON|CTRL_ALLIGN_LEFT, STR_D(ED_KeyYes),       ID_OK, NULL, 14, 4,  0,  0, STR_D(EDH_KeyYes)},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT, STR_D(ED_KeyCancel),ID_CANCEL, NULL, 22, 4},
  {0}
};


/////////////////////////////////////////////////////////////////////////////
#define ID_LF_NAME (ID_USER + 1)
#define ID_LF_STR1 (ID_USER + 2)
#define ID_LF_STR2 (ID_USER + 3)

control dlgFLoad[] = {
  {CTRL_TITLE,                      STR_D(LD_ReloadCaption),             0, NULL,  1, 0, 50, 10},

  {CTRL_STATIC,                     "",                         ID_LF_NAME, NULL,  1, 1, 46},
  {CTRL_STATIC,                     STR_D(LD_FileModified),     ID_LF_STR1, NULL,  1, 3, 46},
  {CTRL_STATIC,                     STR_D(LD_DoYouWantToReload),ID_LF_STR2, NULL,  1, 4, 46},
  {CTRL_LINE,                       "",                                  0, NULL,  1, 6, 46},

  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, STR_D(LD_KeyReload),             ID_OK, NULL, 32, 7,  0,  0, STR_D(LDH_KeyReload)},
  {CTRL_BUTTON   |CTRL_ALLIGN_LEFT, STR_D(LD_KeyNo),             ID_CANCEL, NULL, 41, 7,  0,  0, STR_D(LDH_KeyNo)},
  {0}
};


int FLoadDialog::OnActivate()
{
  if(m_nMode)
  {
    GetItem(0)->SetName(STR_D(LD_RestoreCaption));
    GetItem(ID_LF_STR1)->SetName(STR_D(LD_FileDeleted));
    GetItem(ID_LF_STR2)->SetName(STR_D(LD_DoYouWantToRestore));
    GetItem(ID_OK)->SetName(STR_D(LD_KeyRestore));
    GetItem(ID_OK)->SetHelpLine(STR_D(LDH_KeyRestore));
    GetItem(ID_CANCEL)->SetHelpLine(STR_D(LDH_KeyNoRestore));
  }

  CtrlStatic* pDir = (CtrlStatic*) GetItem(ID_LF_NAME);

  char buff[MAX_PATH + 1];
  short x, y, sizex, sizey;
  pDir->GetPos(&x, &y, &sizex, &sizey);
  SDir::CutPath(m_pName, buff, sizex);
  pDir->SetName(buff);

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
static char sLine[16] = {0};

#define ID_GL_NUMBER (ID_USER + 1)

control dlgGoto[] = {
  {CTRL_TITLE,                      STR_D(GD_GotoCaption),      0, NULL,   1, 0, 34,  7},

  {CTRL_STATIC,                     STR_D(GD_LineNumber),       0, NULL,   1, 1, 15},
  {CTRL_EDIT,                       "",              ID_GL_NUMBER, &sLine,16, 1, 15,  0, STR_D(GDH_InputLine)},
  {CTRL_LINE,                       "",                         0, NULL,   1, 3, 30},

  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, STR_D(GD_KeyGoto),      ID_OK, NULL,  10, 4,  0,  0, STR_D(GDH_KeyGoto)},
  {CTRL_BUTTON   |CTRL_ALLIGN_LEFT, STR_D(GD_KeyCancel),ID_CANCEL, NULL,  21, 4},
  {0}
};


int GotoDialog::OnClose(int id)
{
  //TPRINT(("OnClose id=%x\n", id));
  if(id == ID_OK)
  {
    CtrlEdit* pEdit = (CtrlEdit*) GetItem(ID_GL_NUMBER);
    char buff[15] = {0};

    if(pEdit && pEdit->GetName(buff, sizeof(buff))
    && !ScanDec(buff, 1, 0x7fffffff, &m_nLine))
    {
      TPRINT(("ok  buff=%s n=%d\n", buff, m_nLine));
      return 0;
    }
    else
    {
      TPRINT(("err buff=%s n=%d\n", buff, m_nLine));
      SetErrorLine(STR_D(GDE_BadLineNumber));
      SelectItem(ID_GL_NUMBER);
      Refresh();
      return -1;
    }
  }

  return 0;
}



/////////////////////////////////////////////////////////////////////////////
#define ID_FF_SEARCH   (ID_USER +  1)
#define ID_FF_REPLACE  (ID_USER +  2)
#define ID_FF_SREPLACE (ID_USER +  3)
#define ID_FF_SMASK    (ID_USER +  4)
#define ID_FF_MASK     (ID_USER +  5)
#define ID_FF_PATH     (ID_USER +  6)
#define ID_FF_FILELIST (ID_USER +  7)
#define ID_FF_DIRLIST  (ID_USER +  8)
#define ID_FF_CASE     (ID_USER +  9)
#define ID_FF_REVERSE  (ID_USER + 10)
#define ID_FF_SUBDIR   (ID_USER + 11)
#define ID_FF_OPEN     (ID_USER + 12)
#define ID_FF_PROMPT   (ID_USER + 13)
#define ID_FF_INMARKED (ID_USER + 14)
#define ID_FF_CP       (ID_USER + 15)


control dlgFind[] = {
  {CTRL_TITLE,                      "",                                     0, NULL,              1,  0, 70, 12},

  {CTRL_STATIC,                     STR_D(RD_SearchFor),                    0, NULL,              1,  1, 14},
  {CTRL_COMBO,                      (char*)&SaveFind,            ID_FF_SEARCH, g_sFind,          15,  1, 52,  7, STR_D(RDH_InputSearch)},
  {CTRL_STATIC,                     STR_D(RD_ReplaceWith),     ID_FF_SREPLACE, NULL,              1,  2, 14},
  {CTRL_COMBO,                      (char*)&SaveReplace,        ID_FF_REPLACE, g_sReplace,       15,  2, 52,  7, STR_D(RDH_InputReplace)},

  {CTRL_CHECK,                      STR_D(RD_CaseSensitive),                2, &g_fCaseSensitive, 1,  4,  0,  0, STR_D(RDH_CaseSensitive)},
  {CTRL_CHECK,                      STR_D(RD_WholeWord),                    3, &g_fWholeWord,     1,  5,  0,  0, STR_D(RDH_WholeWord)},
  {CTRL_CHECK,                      STR_D(RD_RestrictInMarked),ID_FF_INMARKED, &g_fInMarked,      1,  6,  0,  0, STR_D(RDH_RestrictInMarked)},

  {CTRL_CHECK,                      STR_D(RD_ReverseDirection), ID_FF_REVERSE, &g_fReverce,      35,  4,  0,  0, STR_D(RDH_ReverseDirection)},
  {CTRL_CHECK,                      STR_D(RD_WithoutPrompt),     ID_FF_PROMPT, &g_fNoPrompt,     35,  4,  0,  0, STR_D(RDH_WithoutPrompt)},

  {CTRL_LINE,                       "",                                     0, NULL,              1,  8, 66},
  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, "",                                 ID_OK, NULL,             43,  9,  0,  0, STR_D(RDH_KeyFind)},
  {CTRL_BUTTON   |CTRL_ALLIGN_LEFT, STR_D(RD_Cancel),               ID_CANCEL, NULL,             57,  9},
  {0}
};


int FindDialog::OnActivate()
{
  Wnd* pWnd = g_WndManager->GetWnd();
  if(pWnd && IS_WNDEDIT(pWnd))
  {
    char buff[MAX_STRLEN];
    WndEdit* pEdit = (WndEdit*) pWnd;
    if(pEdit->GetWord(buff))
    {
      CtrlCombo* pFind = (CtrlCombo*) GetItem(ID_FF_SEARCH);
      char* pStr = pFind->GetSelectedStr(0, NULL);
      if(!pStr || strcmp(buff, pStr))
        pFind->AddStr(0, buff);

      CtrlCombo* pReplace = (CtrlCombo*) GetItem(ID_FF_REPLACE);
      pStr = pReplace->GetSelectedStr(0, NULL);
      if(!pStr || strcmp(buff, pStr))
        pReplace->AddStr(0, buff);
    }

    int first, last;
    pEdit->GetSelectedLines(&first, &last);

    if(!pEdit->IsMarked() || first == last)
    {
      CtrlCheck* pInMarked = (CtrlCheck*) GetItem(ID_FF_INMARKED);
      pInMarked->SetMode(CTRL_DISABLED);
      pInMarked->SetCheck(0);
    }
  }

  if(!m_nMode)
  {
    GetItem(0)->SetName(STR_D(RD_FindCaption));
    GetItem(ID_OK)->SetName(STR_D(RD_KeyFind));

    GetItem(ID_FF_SREPLACE)->SetMode(CTRL_HIDE);
    GetItem(ID_FF_REPLACE)->SetMode(CTRL_HIDE);
    GetItem(ID_FF_PROMPT)->SetMode(CTRL_HIDE);
  }
  else
  {
    GetItem(0)->SetName(STR_D(RD_ReplaceCaption));
    GetItem(ID_OK)->SetName(STR_D(RD_KeyReplace));

    GetItem(ID_FF_REVERSE)->SetMode(CTRL_HIDE);
  }

  return 0;
}


int FindDialog::OnClose(int id)
{
  if(id == ID_OK)
  {
    CtrlCombo* pFind = (CtrlCombo*) GetItem(ID_FF_SEARCH);
    if(!pFind->GetName())
    {
      TPRINT(("err search absent\n"));
      SetErrorLine(STR_D(RDE_SearchStringAbsent));
      SelectItem(ID_FF_SEARCH);
      Refresh();
      return -1;
    }

    if(m_nMode)
    {
      CtrlCombo* pReplace = (CtrlCombo*) GetItem(ID_FF_REPLACE);
      if(!pReplace->GetName())
      {
        TPRINT(("err replace absent\n"));
        SetErrorLine(STR_D(RDE_ReplaceStringAbsent));
        SelectItem(ID_FF_REPLACE);
        Refresh();
        return -1;
      }
    }
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
control dlgFindFile[] = {
  {CTRL_TITLE,                      "",                                 0, NULL,              1,  0, 70, 21},

  {CTRL_STATIC,                     STR_D(RD_SearchFor),                0, NULL,              1,  0, 14},
  {CTRL_COMBO,                      (char*)&SaveFind,        ID_FF_SEARCH, g_sFind,          15,  0, 52,  7, STR_D(RDH_InputSearch)},
  {CTRL_STATIC,                     STR_D(RD_ReplaceWith), ID_FF_SREPLACE, NULL,              1,  1, 14},
  {CTRL_COMBO,                      (char*)&SaveReplace,    ID_FF_REPLACE, g_sReplace,       15,  1, 52,  7, STR_D(RDH_InputReplace)},
  {CTRL_STATIC,                     STR_D(RD_FileMask),       ID_FF_SMASK, NULL,              1,  3, 14},
  {CTRL_COMBO,                      (char*)&SaveFindM,         ID_FF_MASK, NULL,             15,  3, 52,  7, STR_D(RDH_InputMask)},

  {CTRL_STATIC|CTRL_NOCOLOR,        "",                        ID_FF_PATH, NULL,              1,  4, 66},
  {CTRL_LIST,                       STR_D(RD_Directories),  ID_FF_DIRLIST, NULL,              0,  5, 19, 14, STR_D(RDH_Directories)},
  {CTRL_LIST,                       STR_D(RD_Files),       ID_FF_FILELIST, NULL,             19,  5, 34,  8, STR_D(RDH_Files)},

  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, "",                             ID_OK, NULL,             54,  6,  0,  0, STR_D(RDH_KeyFFind)},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT,    STR_D(RD_Cancel),           ID_CANCEL, NULL,             54,  8},
  {CTRL_LINE,                       "",                                 0, NULL,             54, 12, 13},

  {CTRL_CHECK,                      STR_D(RD_CaseSensitive),   ID_FF_CASE, &g_fCaseSensitive,20, 13,  0,  0, STR_D(RDH_CaseSensitive)},
  {CTRL_CHECK,                      STR_D(RD_WholeWord),                3, &g_fWholeWord,    20, 14,  0,  0, STR_D(RDH_WholeWord)},
  {CTRL_CHECK,                      STR_D(RD_Recursive),     ID_FF_SUBDIR, &iRecurs,         20, 15,  0,  0, STR_D(RDH_Recursive)},
  {CTRL_CHECK,                      STR_D(RD_InOpen),          ID_FF_OPEN, &iInOpen,         20, 16,  0,  0, STR_D(RDH_InOpen)},
  {CTRL_CHECK,                      STR_D(RD_WithoutPrompt), ID_FF_PROMPT, &g_fNoPrompt,     20, 17,  0,  0, STR_D(RDH_WithoutPrompt)},

  {CTRL_STATIC,                     STR_D(RD_CP),                       0, NULL,             54, 13, 14},
  {CTRL_SLIST,                      "",                          ID_FF_CP, NULL,             54, 14, 13,  6, STR_D(RDH_CP)},
  {0}
};


int FindFileDialog::OnActivate()
{
  Wnd* pWnd = g_WndManager->GetWnd();
  if(pWnd && IS_WNDEDIT(pWnd))
  {
    char buff[MAX_STRLEN];
    WndEdit* pEdit = (WndEdit*) pWnd;
    if(pEdit->GetWord(buff))
    {
      CtrlCombo* pFind = (CtrlCombo*) GetItem(ID_FF_SEARCH);
      char* pStr = pFind->GetSelectedStr(0, NULL);
      if(!pStr || strcmp(buff, pStr))
        pFind->AddStr(0, buff);

      CtrlCombo* pReplace = (CtrlCombo*) GetItem(ID_FF_REPLACE);
      pStr = pReplace->GetSelectedStr(0, NULL);
      if(!pStr || strcmp(buff, pStr))
        pReplace->AddStr(0, buff);
    }
  }

  if(!m_nMode)
  {
    GetItem(0)->SetName(STR_D(RD_FFindCaption));
    GetItem(ID_OK)->SetName(STR_D(RD_KeyFFind));

    GetItem(ID_FF_SREPLACE)->SetMode(CTRL_HIDE);
    GetItem(ID_FF_REPLACE)->SetMode(CTRL_HIDE);
    GetItem(ID_FF_PROMPT)->SetMode(CTRL_HIDE);

    short x, y, sizex, sizey;
    CtrlStatic* pSMask = (CtrlStatic*)GetItem(ID_FF_SMASK);
    pSMask->GetPos(&x, &y, &sizex, &sizey);
    pSMask->SetPos(x, y - 1);

    CtrlCombo* pMask = (CtrlCombo*)GetItem(ID_FF_MASK);
    pMask->GetPos(&x, &y, &sizex, &sizey);
    pMask->SetPos(x, y - 1);
  }
  else
  {
    GetItem(0)->SetName(STR_D(RD_FReplaceCaption));
    GetItem(ID_OK)->SetName(STR_D(RD_KeyFReplace));
  }

  char* pMask = SaveFindM.GetStr(0);
  if(!pMask)
    pMask = "*.*";
  GetItem(ID_FF_MASK)->SetName(pMask);

  CtrlSList* pCtrl = (CtrlSList*) GetItem(ID_FF_CP);
  for(int i = 0; ; ++i)
  {
    const char* pCP = EnumCPname(i);
    if(!pCP)
      break;
    pCtrl->AppendStr(pCP);
  }
  pCtrl->SetSelect(GetCPindex(TextBuff::s_fileCP));

  char buff[MAX_PATH + 1];
  strcpy_s(buff, sizeof(buff), sFindPath);
  TPRINT(("OnActivate %s\n", buff));
  m_DList.SetFullPath(buff);

  ReadDir();
  return 0;
}


int FindFileDialog::DialogProc(int code)
{
  switch(code >> 16)
  {
  case K_SYMBOL >> 16:
    if(code == K_ENTER)
    {
      if(GetSelectedId() == ID_FF_DIRLIST)
      {
        //directory list selected
        CtrlList* pDList = (CtrlList*) GetItem(ID_FF_DIRLIST);
        int item = pDList->GetSelect();
        size_t l;
        char* pDir = pDList->GetSelectedStr(item, &l);
        char buff[MAX_PATH + 1];
        if(pDir)
        {
          memcpy(buff, pDir, l);
          buff[l] = 0;
        }
        else
          buff[0] = 0;

        m_DList.SetFullPath(buff);
        ReadDir();
        code = 0;
      }
    }
    else if(code == K_BS)
    {
      if(GetSelectedId() == ID_FF_DIRLIST)
      {
        //TPRINT(("BS\n"));
        m_DList.SetFullPath("..");
        ReadDir();
        code = 0;
      }
    }
    break;
  }

  return code;
}


int FindFileDialog::ReadDir()
{
  m_DList.ReadDir();

  CtrlStatic* pDir = (CtrlStatic*) GetItem(ID_FF_PATH);
  char buff[MAX_PATH + 1];
  short x, y, sizex, sizey;
  pDir->GetPos(&x, &y, &sizex, &sizey);
  SDir::CutPath(m_DList.GetPath(), buff, sizex);
  pDir->SetName(buff);

  CtrlList* pFList = (CtrlList*) GetItem(ID_FF_FILELIST);
  pFList->Clear();
  int n;
  for(n = 0; n < MAX_FILES_NUM; ++n)
  {
    FileInfo* pInfo = m_DList.GetFileInfo(n);
    if(!pInfo)
      break;

    int rc = pFList->AppendStr(pInfo->sName);
    if(rc <= 0)
      break;
  }
  TPRINT(("FileList %d\n", n));

  CtrlList* pDList = (CtrlList*) GetItem(ID_FF_DIRLIST);
  pDList->Clear();
  int i;
  for(i = 0; i < MAX_FILES_NUM; ++i)
  {
    FileInfo* pInfo = m_DList.GetDirInfo(i);
    if(!pInfo)
      break;

    pDList->AppendStr(pInfo->sName);
  }

  for(int j = 0; j + i < MAX_FILES_NUM; ++j)
  {
    FileInfo* pInfo = m_DList.GetDrvInfo(j);
    if(!pInfo)
      break;

    pDList->AppendStr(pInfo->sName);
  }

  Refresh();

  return n;
}


int FindFileDialog::OnClose(int id)
{
  if(id == ID_OK)
  {
    CtrlCombo* pFind = (CtrlCombo*) GetItem(ID_FF_SEARCH);
    if(!pFind->GetName())
    {
      TPRINT(("err search absent\n"));
      SetErrorLine(STR_D(RDE_SearchStringAbsent));
      SelectItem(ID_FF_SEARCH);
      Refresh();
      return -1;
    }

    if(m_nMode)
    {
      CtrlCombo* pReplace = (CtrlCombo*) GetItem(ID_FF_REPLACE);
      if(!pReplace->GetName())
      {
        TPRINT(("err replace absent\n"));
        SetErrorLine(STR_D(RDE_ReplaceStringAbsent));
        SelectItem(ID_FF_REPLACE);
        Refresh();
        return -1;
      }
    }

    char buff[MAX_STRLEN];
    CtrlCombo* pCMask = (CtrlCombo*) GetItem(ID_FF_MASK);
    pCMask->GetName(buff, sizeof(buff));
    m_DList.SetFullPath(buff);

    strcpy_s(sFindPath, sizeof(sFindPath), m_DList.GetPath());

    CtrlSList* pCtrl = (CtrlSList*) GetItem(ID_FF_CP);
    TextBuff::s_fileCP = EnumCP(pCtrl->GetSelect());

    char* pMask = SaveFindM.GetStr(0);
    if(!pMask)
      pMask = "*.*";
    SDir::AppendName(sFindPath, pMask, g_InputStr);
    TPRINT(("FindFileDialog::OnClose %s cp=%d\n", g_InputStr, TextBuff::s_fileCP));
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
//scan for matched files
static List* s_pList = NULL;
//number of matched files
static int iFound = 0;
//not add first empty line
static int iFirst = 1;

static int iLPos;//position in matched files list


#define ID_SF_PATH     (ID_USER +  1)
#define ID_SF_FILELIST (ID_USER +  2)
#define ID_SF_PROGRESS (ID_USER +  3)
#define ID_SF_COUNT    (ID_USER +  4)

control dlgSearchFile[] = {
  {CTRL_TITLE,                     STR_D(PD_FileSearchCaption), 0, NULL,  1,  0, 70, 21},

  {CTRL_STATIC,                    STR_D(PD_In),                0, NULL,  1,  0,  4},
  {CTRL_STATIC,                    "",                 ID_SF_PATH, NULL,  5,  0, 62},

  {CTRL_LIST,                      "",             ID_SF_FILELIST, NULL,  0,  1, 68, 17},
  {CTRL_STATIC,                    "",             ID_SF_PROGRESS, NULL,  1, 18,  1},
  {CTRL_STATIC,                    "",                ID_SF_COUNT, NULL,  3, 18, 35},
  //{CTRL_STATIC, STR_D(PD_PressAnyKey),              0, NULL, 40, 18},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT, STR_D(PD_KeyStop),     ID_CANCEL, NULL, 40, 18},
  {0}
};


int SearchFileDialog::Activate()
{
  char* pMask = SaveFindM.GetStr(0);
  if(!pMask)
    pMask = "*.*";
  TPRINT(("SearchFile path=%s mask=%s what=%s\n", sFindPath, pMask, g_sFind));

  m_nActiveView = g_WndManager->GetActiveView();

  AllignButtons();
  Show();
  Refresh();
  CaptureInput();

  iLPos = 0;//new search

  int cs = 0;
  char* pStr = g_sFind;
  char buff[MAX_PATH + 1];
  char* pFind = buff;

  while(*pStr)
  {
    char c = *pStr++;
    wchar wc = char2wchar(g_textCP, c);
    if(!g_fCaseSensitive)
      wc = wc2upper(wc);
    c = wchar2char(TextBuff::s_fileCP, wc);

    cs += c;
    *pFind++ = c;
  }

  *pFind = 0;
  pFind = buff;

  iFound = 0;
  iFirst = 1;

  SetErrorLine(STR_D(PD_PressAnyKey));
  int iKey = ScanDir(sFindPath, pMask, pFind, cs);
  (void)iKey;

#if 0
  //for test only
  g_pApplication->Main(K_CLOSE);
#endif

  //save list
  CtrlList* pList = (CtrlList*) GetItem(ID_SF_FILELIST);
  if(s_pList)
    delete s_pList;
  s_pList = pList->m_pList;
  pList->m_pList = NULL;

  Hide();
  g_WndManager->SetActiveView(m_nActiveView);

  return ID_OK;
}


int SearchFileDialog::ScanDir(char* pPath, char* pMask, char* pFind, int cs)
{
  int iKey = 0, rc = 0;
  int fAppendPath = 0;
  char buff[MAX_PATH + 1];

  CtrlStatic* pSPath = (CtrlStatic*) GetItem(ID_SF_PATH);
  short x, y, sizex, sizey;
  pSPath->GetPos(&x, &y, &sizex, &sizey);
  SDir::CutPath(pPath, buff, sizex);
  pSPath->SetName(buff);

  SDir::AppendName(pPath, pMask, buff);
  DirList DList;
  DList.SetFullPath(buff);
  DList.ScanDir();

  CtrlList* pList = (CtrlList*) GetItem(ID_SF_FILELIST);
  CtrlStatic* pSProgress = (CtrlStatic*) GetItem(ID_SF_PROGRESS);

  for(int i = 0; !iKey; ++i)
  {
    FileInfo* pInfo = DList.GetFileInfo(i);
    if(!pInfo)
      break;

    SDir::AppendName(pPath, pInfo->sName, buff);
    if(ScanFile(buff, pFind, cs))
    {
      if(!fAppendPath)
      {
        fAppendPath = 1;
        if(iFirst)
          iFirst = 0;
        else
          //split line
          rc = pList->AppendStr("");

        if(rc < 0)
          break;
        rc = pList->AppendStr(pPath);
        if(rc < 0)
          break;
      }
      //add 2 space is different from path
      strcpy_s(buff, sizeof(buff), "  ");
      strcat_s(buff, sizeof(buff), pInfo->sName);
      rc = pList->AppendStr(buff);
      if(rc < 0)
        break;

      sprintf_s(buff, sizeof(buff), GetSStr(STR_D(PD_Match_d_files)), ++iFound);
      CtrlStatic* pSCount = (CtrlStatic*) GetItem(ID_SF_COUNT);
      pSCount->SetName(buff);

      pList->SetSelect();
      pList->Refresh();
    }
    iKey = CheckInput();
    ShowProgress(pSProgress);
  }

  if(rc < 0)
    iKey = ID_CANCEL;

  if(iRecurs)
    for(int i = 0; !iKey; ++i)
    {
      FileInfo* pInfo = DList.GetDirInfo(i);
      if(!pInfo)
        break;

      SDir::AppendName(pPath, pInfo->sName, buff);
      iKey = ScanDir(buff, pMask, pFind, cs);
    }

  return iKey;
}


int SearchFileDialog::ScanFile(char* pPath, char* pFind, int cs)
{
  if(iInOpen && !CheckOpenFile(pPath))
    return 0;

  FileObject FObj(pPath);

  return FObj.ScanFile(pFind, cs, g_fCaseSensitive, TextBuff::s_fileCP, g_fWholeWord);
}


int SearchFileDialog::ShowProgress(CtrlStatic* pProgress)
{
  static char progress[] = "-\\|/";
  static int  pos = 0;

  char buff[2];
  buff[0] = progress[pos];
  buff[1] = 0;
  if(++pos >= sizeof(progress) - 1)
    pos = 0;

  pProgress->SetName(buff);

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
#define ID_MF_COUNT    (ID_USER +  1)
#define ID_MF_FILELIST (ID_USER +  2)

control dlgMatchedFile[] = {
  {CTRL_TITLE,                      STR_D(MD_MatchedFilesCaption), 0, NULL,   1,  0, 70, 21},

  {CTRL_LIST,                       "",               ID_MF_FILELIST, &iLPos, 0,  0, 68, 18, STR_D(MDH_SelectFile)},
  {CTRL_STATIC,                     "",                  ID_MF_COUNT, NULL,   1, 18, 47},

  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, STR_D(MD_Open),            ID_OK, NULL,  40, 18,  0,  0, STR_D(MDH_Open)},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT,    STR_D(MD_Cancel),      ID_CANCEL, NULL,  50, 18},
  {0}
};


int MatchedFileDialog::OnActivate()
{
  if(!iFound)
  {
    BeginPaint();
    SetErrorLine(STR_D(MDE_MatchedFilesAbsent));
    StopPaint();
    return 1;
  }

  CtrlStatic* pSCount = (CtrlStatic*) GetItem(ID_MF_COUNT);
  char buff[128];
  sprintf_s(buff, sizeof(buff), GetSStr(STR_D(MD_s_found_in_d_files)), g_sFind, iFound);
  pSCount->SetName(buff);

  if(s_pList)
  {
    //sel local list
    CtrlList* pList = (CtrlList*) GetItem(ID_MF_FILELIST);
    delete pList->m_pList;
    pList->m_pList = s_pList;
    pList->SetSelect(iLPos);
  }

  return 0;
}


int MatchedFileDialog::OnClose(int id)
{
  CtrlList* pList = (CtrlList*) GetItem(ID_MF_FILELIST);
  if(id == ID_OK)
  {
    int n = pList->GetSelect();
    size_t l;
    char* pName = pList->GetSelectedStr(n, &l);
    if(!pName || !l || *pName != ' ')
      goto ERR;

    //skip 2 space before name
    char name[MAX_PATH + 1];
    memcpy(name, pName + 2, l - 2);
    name[l - 2] = 0;

    char* pPath = NULL;
    for(int i = n - 1; i >= 0; --i)
    {
      pPath = pList->GetSelectedStr(i, &l);
      if(pPath && l && *pPath != ' ')
        break;
    }
    memcpy(g_InputStr, pPath, l);
    g_InputStr[l] = 0;

    SDir::AppendName(g_InputStr, name, g_InputStr);
    TPRINT(("Select file %s\n", g_InputStr));
  }

  //clear list
  if(pList->m_pList == s_pList)
    pList->m_pList = NULL;

  return 0;

ERR:
  SelectItem(ID_MF_FILELIST);
  Refresh();
  return -1;
}


/////////////////////////////////////////////////////////////////////////////
#define ID_WL_WNDLIST  (ID_USER +  1)
#define ID_WL_COUNT    (ID_USER +  2)
#define ID_WL_CLOSE    (ID_USER +  3)

control dlgWindowList[] = {
  {CTRL_TITLE,                      STR_D(WD_WindowListCaption),  0, NULL,  1,  0, 70, 21},

  {CTRL_LIST,                       "",               ID_WL_WNDLIST, NULL,  0,  0, 68, 18, STR_D(WDH_SelectWindow)},
  {CTRL_STATIC,                     "",                 ID_WL_COUNT, NULL,  1, 18, 35},

  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, STR_D(WD_KeySelect),      ID_OK, NULL, 36, 18,  0,  0, STR_D(WDH_KeySelect)},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT,    STR_D(WD_KeyClose), ID_WL_CLOSE, NULL, 47, 18,  0,  0, STR_D(WDH_KeyClose)},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT,    STR_D(WD_KeyCancel),  ID_CANCEL, NULL, 57, 18},
  {0}
};


int WindowListDialog::ListWnd(int skip)
{
  TPRINT(("ListWnd\n"));
  Wnd* pWnd = g_WndManager->GetWnd(0, 0);
  if(!pWnd)
    return 0;

  CtrlList* pList = (CtrlList*) GetItem(ID_WL_WNDLIST);
  short x, y, sizex, sizey;
  pList->GetPos(&x, &y, &sizex, &sizey);

  pList->Clear();
  m_WndList.Clear();

  FileInfo FlInfo;
  memset(&FlInfo, 0, sizeof(FlInfo));

  int wnd   = 0;
  int count = 0;

  if(skip)
    pWnd = pWnd->m_pNext;

  if(!m_nActiveView && (m_nMode == 1 || m_nMode == 2))
  {
    pWnd = pWnd->m_pNext;
    ++wnd;
  }

  while(pWnd)
  {
    const char* pName = pWnd->GetObjPath();

    FlInfo.nSize = wnd;
    FlInfo.p     = pWnd;
    strcpy_s(FlInfo.sName, sizeof(FlInfo.sNameExt) + 1, pName);

    if(!m_nMode || m_nMode == 3 || (IS_WNDEDIT(pWnd) && ((WndEdit*)pWnd)->IsMarked()))
    {
      m_WndList.AddItem(&FlInfo);
      ++count;
    }
    ++wnd;
    pWnd = pWnd->m_pNext;
  }

  int fClone = 0;
  pWnd = g_WndManager->GetWnd(0, 1);
  if(pWnd)
  {
    if((!m_nMode && !pWnd->IsClone()) || (!m_nActiveView && (m_nMode == 1 || m_nMode == 2)))
    {
      const char* pName = pWnd->GetObjPath();

      FlInfo.nSize = -2;
      FlInfo.p     = pWnd;
      strcpy_s(FlInfo.sName, sizeof(FlInfo.sNameExt) + 1, pName);

      if(!m_nMode || m_nMode == 3 || (IS_WNDEDIT(pWnd) && ((WndEdit*)pWnd)->IsMarked()))
      {
        m_WndList.AddItem(&FlInfo);
        ++count;
      }
    }
    else
      fClone = 1;
  }

  TPRINT(("s=%d w=%d c=%d cl=%d\n", skip, wnd, count, fClone));

  if(!count)
    return 0;

  if(count > 1)
    m_WndList.Sort(byName);

  int select = -1;
  for(int i = 0; i < count; ++i)
  {
    FileInfo* pInfo = m_WndList.GetItem(i);
    pWnd = (Wnd*) pInfo->p;

    const char* pPath = pWnd->GetObjPath();

    char path[MAX_PATH + 1];
    SDir::GetRelPath(pPath, path, sizeof(path));

    char buff[MAX_PATH + 3];
    SDir::CutPath(path, buff + 2, sizex - 4);
    if((int)pInfo->nSize >= 0)
      buff[0] = pWnd->GetAccessInfo();
    else
      buff[0] = '>';
    buff[1] = ' ';
    pList->AppendStr(buff);

    if(!skip)
      if(((!m_nActiveView || fClone) && !pInfo->nSize)
      ||  ( m_nActiveView       && (int) pInfo->nSize < 0))
        select = i;
  }

  if(select != -1)
    pList->SetSelect(select);

  return count;
}


int WindowListDialog::OnActivate()
{
  //TPRINT(("WindowListDialog::OnActivate\n"));

  Wnd* pWnd = g_WndManager->GetWnd();
  if(!pWnd)
  {
    BeginPaint();
    SetErrorLine(STR_D(WDE_AllWindowsClosed));
    StopPaint();
    return 1;
  }

  int skip = 0;
  if(m_nMode == 3)
    skip = 1;

  int count;
  if((count = ListWnd(skip)) == 0)
  {
    BeginPaint();
    SetErrorLine(STR_D(WDE_NoMarkedBlock));
    StopPaint();
    return 1;
  }

  if(m_nMode == 1)
  {
    GetItem(0)->SetName(STR_D(WD_WindowCopyCaption));
    GetItem(ID_OK)->SetName(STR_D(WD_KeyCopy));
    GetItem(ID_OK)->SetHelpLine(STR_D(WDH_KeyCopy));
    GetItem(ID_WL_CLOSE)->SetName(STR_D(WD_KeyUnmark));
    GetItem(ID_WL_CLOSE)->SetHelpLine(STR_D(WDH_KeyUnmark));
  }
  else if(m_nMode == 2)
  {
    GetItem(0)->SetName(STR_D(WD_WindowMoveCaption));
    GetItem(ID_OK)->SetName(STR_D(WD_KeyMove));
    GetItem(ID_OK)->SetHelpLine(STR_D(WD_KeyMove));
    GetItem(ID_WL_CLOSE)->SetName(STR_D(WD_KeyUnmark));
    GetItem(ID_WL_CLOSE)->SetHelpLine(STR_D(WDH_KeyUnmark));
  }
  else if(m_nMode == 3)
  {
    GetItem(0)->SetName(STR_D(WD_WindowDiffCaption));
    GetItem(ID_OK)->SetHelpLine(STR_D(WDH_KeySelectDiff));
    GetItem(ID_WL_CLOSE)->SetMode(CTRL_HIDE);
  }

  CtrlStatic* pSCount = (CtrlStatic*) GetItem(ID_WL_COUNT);
  char buff[128];
  sprintf_s(buff, sizeof(buff), GetSStr(STR_D(WDS_d_Windows)), count);
  pSCount->SetName(buff);

  return 0;
}


int WindowListDialog::DialogProc(int code)
{
  if(code == ID_WL_CLOSE || code == K_DELETE || code == '-')
  {
    code = 0;

    CtrlList* pList = (CtrlList*) GetItem(ID_WL_WNDLIST);
    int n = pList->GetSelect();
    //TPRINT(("Close %d windows\n", n));

    FileInfo* pInfo = m_WndList.GetItem(n);
    if(!pInfo)
      return 0;

    int wnd = (int)pInfo->nSize;

    Wnd* pWnd = g_WndManager->GetWnd(wnd + 1);
    if(!m_nMode)
    {
      pWnd->Close();
      if(wnd < 0)
        m_nActiveView = 0;
    }
    else
    {
      ((WndEdit*)pWnd)->SelectClear();
      g_WndManager->Invalidate();
    }

    int count;
    if((count = ListWnd(1)) == 0) //skip dialog
    {
      GetItem(ID_WL_WNDLIST)->SetMode(CTRL_DISABLED);
      GetItem(ID_OK)        ->SetMode(CTRL_DISABLED);
      GetItem(ID_WL_CLOSE)  ->SetMode(CTRL_DISABLED);
      SelectItem(ID_CANCEL);
      if(!m_nMode)
      {
        GetItem(ID_CANCEL)->SetHelpLine(STR_D(WDE_AllWindowsClosed));
        SetErrorLine(STR_D(WDE_AllWindowsClosed));
      }
      else
      {
        GetItem(ID_CANCEL)->SetHelpLine(STR_D(WDE_NoMarkedBlock));
        SetErrorLine(STR_D(WDE_NoMarkedBlock));
      }
    }
    else
      pList->SetSelect(n);

    CtrlStatic* pSCount = (CtrlStatic*) GetItem(ID_WL_COUNT);
    char buff[128];
    sprintf_s(buff, sizeof(buff), GetSStr(STR_D(WDS_d_Windows)), count);
    pSCount->SetName(buff);
  }

  return code;
}


int WindowListDialog::OnClose(int id)
{
  if(id == ID_OK)
  {
    //уберем свое окно из списка
    g_WndManager->DelWnd(this);

    CtrlList* pList = (CtrlList*) GetItem(ID_WL_WNDLIST);
    int n = pList->GetSelect();
    //TPRINT(("Select %d windows\n", n));

    FileInfo* pInfo = m_WndList.GetItem(n);
    int wnd = (int)pInfo->nSize;

    if(m_nMode == 0)
    {
      TPRINT(("wnd=%d view=%d\n", wnd, m_nActiveView));
      g_WndManager->SetTopWnd(wnd, m_nActiveView);
    }
    else if(m_nMode == 1)
    {
      //window copy
      WndEdit* pFrom = (WndEdit*) g_WndManager->GetWnd(wnd);
      WndEdit* pTo   = (WndEdit*) g_WndManager->GetWnd(0, m_nActiveView);
      if(pFrom && pTo)
        pTo->EditWndCopy(pFrom);
    }
    else if(m_nMode == 2)
    {
      //window move
      WndEdit* pFrom = (WndEdit*) g_WndManager->GetWnd(wnd);
      WndEdit* pTo   = (WndEdit*) g_WndManager->GetWnd(0, m_nActiveView);
      if(pFrom && pTo)
        pTo->EditWndMove(pFrom);
    }
    else if(m_nMode == 3)
    {
      TPRINT(("Select Wnd %d\n", wnd));
      //skip 2 dialogs
      WndEdit* pWnd = (WndEdit*) g_WndManager->GetWnd(wnd + 1);
      g_WndManager->ChangeViewMode();
      g_WndManager->SetTopWnd(pWnd, 1);
    }
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
#define ID_FL_FUNCLIST (ID_USER +  1)
#define ID_FL_COUNT    (ID_USER +  2)

control dlgFuncList[] = {
  {CTRL_TITLE,                      STR_D(FLD_FunctionListCaption), 0, NULL,  1,  0, 70, 21},

  {CTRL_LIST,                       "",                ID_FL_FUNCLIST, NULL,  0,  0, 68, 18, STR_D(FLDH_FunctionList)},
  {CTRL_STATIC,                     "",                   ID_FL_COUNT, NULL,  1, 18, 35},

  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, STR_D(FLD_KeyGoto),         ID_OK, NULL, 36, 18,  0,  0, STR_D(FLDH_KeyGoto)},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT,    STR_D(FLD_KeyCancel),   ID_CANCEL, NULL, 57, 18},
  {0}
};


int FuncListDialog::OnActivate()
{
  //change list
  CtrlList* pList = (CtrlList*) GetItem(ID_FL_FUNCLIST);
  delete pList->m_pList;
  pList->m_pList = m_pList;
  pList->SetSelect(m_nLine);

  int count = (int)pList->GetStrCount();
  CtrlStatic* pSCount = (CtrlStatic*) GetItem(ID_FL_COUNT);
  char buff[128];
  sprintf_s(buff, sizeof(buff), GetSStr(STR_D(FLD_d_funstions)), count);
  pSCount->SetName(buff);
  return 0;
}


int FuncListDialog::OnClose(int id)
{
  //clear list
  CtrlList* pList = (CtrlList*) GetItem(ID_FL_FUNCLIST);

  if(id == ID_OK)
  {
    int n = pList->GetSelect();
    size_t l;
    char* pFunc = pList->GetSelectedStr(n, &l);

    m_nLine = -1;
    sscanf_s(pFunc, "%d", &m_nLine);
    if(m_nLine > 0)
      --m_nLine;
    //TPRINT(("Func line %d\n", line));
  }

  pList->m_pList = NULL;
  return 0;
}



/////////////////////////////////////////////////////////////////////////////
#define ID_DF_FILE1        (ID_USER + 1)
#define ID_DF_FILE2        (ID_USER + 2)
#define ID_DF_WHOLE        (ID_USER + 3)
#define ID_DF_FROMCUR      (ID_USER + 4)
#define ID_DF_INMARKED     (ID_USER + 5)
#define ID_DF_WITHOUTSPACE (ID_USER + 6)

static int iDiffRange = 0;
static int iDiffWithoutSpace;


control dlgDiff[] = {
  {CTRL_TITLE,                      STR_D(DFD_DifferFileCaption),                        0, NULL,               1,  0, 70, 13},

  {CTRL_STATIC,                     STR_D(DFD_Compare),                                  0, NULL,               1,  1,  9},
  {CTRL_STATIC,                     "",                                        ID_DF_FILE1, NULL,              10,  1, 57,  7},
  {CTRL_STATIC,                     STR_D(DFD_With),                                     0, NULL,               1,  2,  9},
  {CTRL_STATIC,                     "",                                        ID_DF_FILE2, NULL,              10,  2, 57,  7},

  {CTRL_LINE,                       "",                                                  0, NULL,               1,  4, 66},
  {CTRL_RADIO,                      STR_D(DFD_CompareWholeFiles),              ID_DF_WHOLE, &iDiffRange,        1,  5,  0,  0, STR_D(DFDH_CompareWholeFiles)},
  {CTRL_RADIO,                      STR_D(DFD_CompareFromCurrentPosition),   ID_DF_FROMCUR, &iDiffRange,        1,  6,  0,  0, STR_D(DFDH_CompareFromCurrentPosition)},
  {CTRL_RADIO,                      STR_D(DFD_RestrictInMarkedBlock),       ID_DF_INMARKED, &iDiffRange,        1,  7,  0,  0, STR_D(DFDH_RestrictInMarkedBlock)},
  {CTRL_CHECK,                      STR_D(DFD_IgnoreDifferencesInSpace),ID_DF_WITHOUTSPACE, &iDiffWithoutSpace,35,  5,  0,  0, STR_D(DFDH_IgnoreDifferencesInSpace)},
  {CTRL_CHECK,                      STR_D(DFD_MarkDifferOnExit),                         0, &g_fMarkDiff,      35,  6,  0,  0, STR_D(DFDH_MarkDifferOnExit)},

  {CTRL_LINE,                       "",                                                  0, NULL,               1,  9, 66},
  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, STR_D(DFD_KeyCompare),                           ID_OK, NULL,              40, 10,  0,  0, STR_D(DFDH_KeyCompare)},
  {CTRL_BUTTON   |CTRL_ALLIGN_LEFT, STR_D(DFD_KeyCancel),                        ID_CANCEL, NULL,              50, 10},
  {0}
};



int DiffDialog::OnActivate()
{
  CtrlStatic* pFile1 = (CtrlStatic*) GetItem(ID_DF_FILE1);

  short x, y, sizex, sizey;
  pFile1->GetPos(&x, &y, &sizex, &sizey);

  WndEdit* pWnd1 = (WndEdit*) g_WndManager->GetWnd(0, 0);

  const char* pPath = pWnd1->GetObjPath();
  char path[MAX_PATH + 1];
  SDir::GetRelPath(pPath, path, sizeof(path));

  char name[MAX_PATH + 1];
  SDir::CutPath(path, name, sizex);

  pFile1->SetName(name);

  WndEdit* pWnd2 = (WndEdit*) g_WndManager->GetWnd(0, 1);
  if(!pWnd2)
  {
    TPRINT(("DiffDialog::OnActivate create VIEW\n"));
    WindowListDialog Dlg(3);
    int rc = Dlg.Activate();
    if(rc != ID_OK)
    {
      SetErrorLine(STR_D(DFDE_SelectWindowForComparing));
      return 1;
    }
  }

  pWnd2 = (WndEdit*) g_WndManager->GetWnd(0, 1);
  CtrlStatic* pFile2 = (CtrlStatic*) GetItem(ID_DF_FILE2);

  pPath = pWnd2->GetObjPath();
  SDir::GetRelPath(pPath, path, sizeof(path));
  SDir::CutPath(path, name, sizex);

  pFile2->SetName(name);

  if(!pWnd1->IsMarked() || !pWnd2->IsMarked())
  {
    CtrlRadio* pCtrl = (CtrlRadio*) GetItem(ID_DF_INMARKED);
    if(pCtrl->GetCheck())
    {
      ((CtrlRadio*) GetItem(ID_DF_WHOLE))->SetCheck();
      SelectItem(ID_DF_WHOLE);
    }
    pCtrl->SetMode(CTRL_DISABLED);
  }

  return 0;
}


int DiffDialog::OnClose(int id)
{
  if(id == ID_OK)
  {
    WndEdit* pWnd1 = (WndEdit*) g_WndManager->GetWnd(1);
    WndEdit* pWnd2 = (WndEdit*) g_WndManager->GetWnd(-1);

    if(pWnd1 && pWnd2 && IS_WNDEDIT(pWnd1) && IS_WNDEDIT(pWnd2))
    {
      int first1 = 0;
      int first2 = 0;
      int last1  = -1;
      int last2  = -1;

      if(((CtrlRadio*) GetItem(ID_DF_FROMCUR))->GetCheck())
      {
        first1 = (int)pWnd1->GetCurLine();
        first2 = (int)pWnd2->GetCurLine();
      }
      else if(((CtrlRadio*) GetItem(ID_DF_INMARKED))->GetCheck())
      {
        pWnd1->GetSelectedLines(&first1, &last1);
        pWnd2->GetSelectedLines(&first2, &last2);
      }

      ((CtrlCheck*) GetItem(ID_DF_WITHOUTSPACE))->UpdateVar();
      m_pDiff = new Diff(pWnd1->GetTextBuff(), pWnd2->GetTextBuff(), iDiffWithoutSpace,
        first1, first2, last1, last2);

      m_pDiff->Compare();
    }
  }

  return 0;
}


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

control dlgProperties[] = {
  {CTRL_TITLE,                      STR_D(PRD_FilePropertiesCaption),      0, NULL,               1,  0, 70, 17},

  {CTRL_STATIC,                     "",                           ID_DP_PATH, NULL,               1,  1, 66,  7},
  {CTRL_STATIC,                     "",                           ID_DP_NAME, NULL,               1,  2, 66,  7},
  {CTRL_STATIC,                     "",                           ID_DP_INFO, NULL,               1,  3, 66,  7},
  {CTRL_LINE,                       "",                                    0, NULL,               1,  5, 66},

  {CTRL_STATIC,                     STR_D(PRD_ColorScheme),                0, NULL,               1,  6, 14},
  {CTRL_SLIST,                      "",                          ID_DP_COLOR, &iType,            15,  6, 17,  7, STR_D(PRDH_ColorScheme)},
  {CTRL_STATIC,                     STR_D(PRD_CodePage),                   0, NULL,               1,  7, 14},
  {CTRL_SLIST,                      "",                             ID_DP_CP, NULL,              15,  7, 17,  7, STR_D(PRDH_CodePage)},

  {CTRL_CHECK,                      STR_D(PRD_ReadOnly),            ID_DP_RO, &g_WndProp.ro,      1,  9, 30,  1, STR_D(PRDH_ReadOnly)},
  {CTRL_CHECK,                      STR_D(PRD_LogFile),            ID_DP_LOG, &g_WndProp.log,     1, 10, 30,  1, STR_D(PRDH_LogFile)},

  {CTRL_STATIC,                     STR_D(PRD_EndLine),                    0, NULL,              35,  6, 13},
  {CTRL_SLIST,                      "",                           ID_DP_CRLF, NULL,              50,  6, 17,  6, STR_D(PRDH_EndLine)},

  {CTRL_STATIC,                     STR_D(PRD_TabSize),                    0, NULL,              35,  8, 13},
  {CTRL_EDIT,                       "",                            ID_DP_TAB, NULL,              65,  8,  2,  7, STR_D(PRDH_TabSize)},
  {CTRL_RADIO,                      STR_D(PRD_TabsToSpace),ID_DP_TAB_CONVERT, &g_WndProp.savetab,35,  9, 30,  1, STR_D(PRDH_TabsToSpace)},
  {CTRL_RADIO,                      STR_D(PRD_UseTabs),       ID_DP_TAB_SAVE, &g_WndProp.savetab,35, 10, 30,  1, STR_D(PRDH_UseTabs)},
  {CTRL_CHECK,                      STR_D(PRD_ShowTabs),      ID_DP_TAB_SHOW, &g_WndProp.showtab,35, 11, 30,  1, STR_D(PRDH_ShowTabs)},

  {CTRL_LINE,                       "",                                    0, NULL,               1, 13, 66},
  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, STR_D(PRD_KeyOK),                  ID_OK, NULL,              50, 14,  0,  0, STR_D(PRDH_KeyOK)},
  {CTRL_BUTTON   |CTRL_ALLIGN_LEFT, STR_D(PRD_KeyCancel),          ID_CANCEL, NULL,              60, 14},
  {0}
};


int PropertiesDialog::OnActivate()
{
  CtrlStatic* pPath = (CtrlStatic*) GetItem(ID_DP_PATH);

  short x, y, sizex, sizey;
  pPath->GetPos(&x, &y, &sizex, &sizey);

  char* pFile = g_WndProp.path;
  char buff[MAX_PATH + 1];

  if(strlen(pFile) <= sizex)
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

    pPath = (CtrlStatic*) GetItem(ID_DP_NAME);
    *pName = c;
    SDir::CutPath(pName, buff, sizex);
    pPath->SetName(buff);
  }

  CtrlStatic* pInfo = (CtrlStatic*) GetItem(ID_DP_INFO);


  char ts[128];
  int rc = ctime_s(ts, sizeof(ts), &g_WndProp.time);
  assert(rc == 0);
  (void)rc;

  char* pTime = ts;
  while(*pTime != ' ')
    //skip week day
    ++pTime;
  while(*pTime == ' ')
    //skip week day
    ++pTime;
  //del EOL
  pTime[strlen(pTime) - 1] = 0;

  sprintf_s(buff, sizeof(buff), GetSStr(STR_D(PRDS_Bytes)), g_WndProp.size, pTime);
  pInfo->SetName(buff);

  CtrlSList* pCtrl = (CtrlSList*) GetItem(ID_DP_COLOR);
  size_t jeton = 0;
  const char* pLex;
  while((pLex = g_LexCfg.Enum(&jeton)) != NULL)
    pCtrl->AppendStr((char*) pLex);
  pCtrl->SetSelect(g_LexCfg.GetCfgN(g_WndProp.parsemode));

  pCtrl = (CtrlSList*) GetItem(ID_DP_CRLF);
  pCtrl->AppendStr("UNIX  (LF)");
  pCtrl->AppendStr("DOS   (CR+LF)");
  pCtrl->AppendStr("MAC   (CR)");
  pCtrl->SetSelect(g_WndProp.crlf);

  pCtrl = (CtrlSList*) GetItem(ID_DP_CP);
  for(int i = 0; ; ++i)
  {
    const char* pCP = EnumCPname(i);
    if(!pCP)
      break;
    pCtrl->AppendStr(pCP);
  }
  pCtrl->SetSelect(GetCPindex(g_WndProp.cp));

  CtrlEdit* pTab = (CtrlEdit*) GetItem(ID_DP_TAB);
  sprintf_s(buff, sizeof(buff), "%d", g_WndProp.tabsize);
  pTab->SetName(buff);

  return 0;
}


int PropertiesDialog::OnClose(int id)
{
  if(id == ID_OK)
  {
    char buff[15] = {0};
    int tab = 0;

    CtrlEdit* pEdit = (CtrlEdit*) GetItem(ID_DP_TAB);
    if(pEdit && pEdit->GetName(buff, sizeof(buff))
    && ScanDec(buff, 1, 10, &tab))
    {
      TPRINT(("err buff=%s tab=%d\n", buff, tab));
      SetErrorLine(STR_D(PRDE_BadTabSize));
      SelectItem(ID_DP_TAB);
      Refresh();
      return -1;
    }

    g_WndProp.tabsize   = tab;

    CtrlSList* pCtrl = (CtrlSList*) GetItem(ID_DP_CP);
    g_WndProp.cp = EnumCP(pCtrl->GetSelect());

    pCtrl = (CtrlSList*) GetItem(ID_DP_COLOR);
    g_WndProp.parsemode = g_LexCfg.GetCfgName(pCtrl->GetSelect());

    pCtrl = (CtrlSList*) GetItem(ID_DP_CRLF);
    g_WndProp.crlf = pCtrl->GetSelect();
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
static int ra_pos;

#define ID_RA_LIST      (ID_USER +  1)
#define ID_RA_ADD       (ID_USER +  2)
#define ID_RA_DELETE    (ID_USER +  3)
#define ID_RA_DELALL    (ID_USER +  4)

#define ID_RA_LINE1     (ID_USER +  5)
#define ID_RA_LINE2     (ID_USER +  6)
#define ID_RA_LINE3     (ID_USER +  7)
#define ID_RA_LINE4     (ID_USER +  8)

control dlgRAccess[] = {
  {CTRL_TITLE,                      STR_D(RAD_RandomAccessListCaption), 0, NULL,    1,  0, 70, 21},

  {CTRL_LIST,                       "",                        ID_RA_LIST, &ra_pos, 0,  0, 68, 12, STR_D(RADH_SelectRandomAccessPosition)},

  {CTRL_STATIC|CTRL_NOCOLOR,        "",                       ID_RA_LINE1, NULL,    1, 12, 66},
  {CTRL_STATIC|CTRL_NOCOLOR,        "",                       ID_RA_LINE2, NULL,    1, 13, 66},
  {CTRL_STATIC|CTRL_NOCOLOR,        "",                       ID_RA_LINE3, NULL,    1, 14, 66},
  {CTRL_STATIC|CTRL_NOCOLOR,        "",                       ID_RA_LINE4, NULL,    1, 15, 66},
  {CTRL_LINE,                       "",                                 0, NULL,    1, 17, 66},

  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, STR_D(RAD_KeyGoto),             ID_OK, NULL,   17, 18,  0,  0, STR_D(RADH_KeyGoto)},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT,    STR_D(RAD_KeyAdd),          ID_RA_ADD, NULL,   27, 18,  0,  0, STR_D(RADH_KeyAdd)},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT,    STR_D(RAD_KeyDelete),    ID_RA_DELETE, NULL,   37, 18,  0,  0, STR_D(RADH_KeyDelete)},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT,    STR_D(RAD_KeyDeleteAll), ID_RA_DELALL, NULL,   47, 18,  0,  0, STR_D(RADH_KeyDeleteAll)},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT,    STR_D(RAD_KeyCancel),       ID_CANCEL, NULL,   57, 18},
  {0}
};


int RndAccessDialog::FillLines(int pos)
{
  CtrlStatic* pLine = (CtrlStatic*) GetItem(ID_RA_LINE1);
  short x, y, sizex, sizey;
  pLine->GetPos(&x, &y, &sizex, &sizey);

  if(!RAccess[pos].pTBuff)
  {
    for(int i = 0; i < 4; ++i)
    {
      pLine = (CtrlStatic*) GetItem(ID_RA_LINE1 + i);
      pLine->SetName("");
    }
  }
  else
  {
    char buff[MAX_PATH + 1];
    int line = RAccess[pos].y;

    for(int i = 0; i < 4; ++i)
    {
      wchar* pStr = RAccess[pos].pTBuff->GetStr(line + i);
      //size_t len = RAccess[pos].pTBuff->GetStrLen(pStr);

      sprintf_s(buff, sizeof(buff), "%3d: ", line + i + 1);
      size_t off = strlen(buff);
      for(x = 0; x < sizex - off; ++x)
        buff[off + x] = wchar2char(g_textCP, pStr[x]);
      buff[off + x] = 0;

      pLine = (CtrlStatic*) GetItem(ID_RA_LINE1 + i);
      pLine->SetName(buff);
    }
  }

  return 0;
}


int RndAccessDialog::OnActivate()
{
  CtrlList* pList = (CtrlList*) GetItem(ID_RA_LIST);
  short x, y, sizex, sizey;
  pList->GetPos(&x, &y, &sizex, &sizey);

  InitRAccess();

  for(int i = 0; i < NUM_RND_ACCESS; ++i)
  {
    char buff[MAX_PATH + 1];
    sprintf_s(buff, sizeof(buff), "%d:", i);

    if(RAccess[i].pTBuff)
    {
      const char* pPath = RAccess[i].pTBuff->GetObjPath();

      char path[MAX_PATH + 1];
      SDir::GetRelPath(pPath, path, sizeof(path));

      SDir::CutPath(path, buff + 2, sizex - 3);
      pList->AppendStr(buff);
    }
    else
    {
      pList->AppendStr(buff);
    }
  }

  pList->SetSelect(ra_pos);
  FillLines(ra_pos);

  return 0;
}


int RndAccessDialog::DialogProc(int code)
{
  if((code & K_TYPEMASK) == K_SELECT && GetSelectedId() == ID_RA_LIST)
  {
    //list selected
    int item = code & K_CODEMASK;
    FillLines(item);
  }
  else if(code == ID_RA_ADD || code == '+')
  {
    CtrlList* pList = (CtrlList*) GetItem(ID_RA_LIST);
    short x, y, sizex, sizey;
    pList->GetPos(&x, &y, &sizex, &sizey);

    int pos = pList->GetSelect();

    Wnd* pWnd = g_WndManager->GetWnd(1, m_nActiveView);
    if(pWnd && IS_WNDEDIT(pWnd))
    {
      WndEdit* pEWnd = (WndEdit*) pWnd;
      TextBuff* pTBuff = pEWnd->GetTextBuff();
      AddRAccess(pos, pEWnd);
      FillLines(pos);

      //TPRINT(("Add to pos=%d x=%d y=%d %s\n",
      //  pos, RAccess[pos].x, RAccess[pos].y, pTBuff->GetObjPath()));

      char buff[MAX_PATH + 1];
      sprintf_s(buff, sizeof(buff), "%d:", pos);

      const char* pPath = pTBuff->GetObjPath();

      char path[MAX_PATH + 1];
      SDir::GetRelPath(pPath, path, sizeof(path));

      SDir::CutPath(path, buff + 2, sizex - 3);
      pList->ChangeStr(pos, buff);
      Refresh();
    }

    code = 0;
  }
  else if(code == ID_RA_DELETE || code == K_DELETE || code == '-')
  {
    CtrlList* pList = (CtrlList*) GetItem(ID_RA_LIST);

    int pos = pList->GetSelect();

    if(RAccess[pos].pTBuff)
    {
      //TPRINT(("Del pos=%d x=%d y=%d %s\n",
      //  pos, RAccess[pos].x, RAccess[pos].y, RAccess[pos].pTBuff->GetObjPath()));

      DelRAccess(pos);
      FillLines(pos);

      char buff[MAX_PATH + 1];
      sprintf_s(buff, sizeof(buff), "%d:", pos);

      pList->ChangeStr(pos, buff);
      Refresh();
    }

    code = 0;
  }
  else if(code == ID_RA_DELALL)
  {
    CtrlList* pList = (CtrlList*) GetItem(ID_RA_LIST);

    ClrRAccess();
    FillLines(0);

    char buff[MAX_PATH + 1];
    for(int i = 0; i < NUM_RND_ACCESS; ++i)
    {
      sprintf_s(buff, sizeof(buff), "%d:", i);
      pList->ChangeStr(i, buff);
    }

    Refresh();
    code = 0;
  }
  else if(code >= '0' && code <= '9')
  {
    code -= '0';
    CtrlList* pList = (CtrlList*) GetItem(ID_RA_LIST);
    pList->SetSelect(code);

    Refresh();
    code = 0;
  }

  return code;
}


int RndAccessDialog::OnClose(int id)
{
  if(id == ID_OK)
  {
    CtrlList* pList = (CtrlList*) GetItem(ID_RA_LIST);

    int pos = pList->GetSelect();

    if(!RAccess[pos].pTBuff)
    {
      //TPRINT(("err select %d\n", pos));
      //SetErrorLine("Empty random access position selected");
      SelectItem(ID_RA_ADD);
      Refresh();
      return -1;
    }

    //уберем свое окно из списка
    g_WndManager->DelWnd(this);

    GotoRAccess(pos, m_nActiveView);
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
static int nKey;
static int nMod;

control dlgKeyGen[] = {
  {CTRL_TITLE,                      "Key generation", 0, NULL,   1,  0, 60, 19},

  {CTRL_RADIO,                      "N&one",          0, &nMod,  1,  1},
  {CTRL_RADIO,                      "&Shift",         0, &nMod, 15,  1},
  {CTRL_RADIO,                      "&Control",       0, &nMod, 30,  1},
  {CTRL_RADIO,                      "&Alt",           0, &nMod, 45,  1},
  {CTRL_LINE,                       "",               0, NULL,   1,  3, 56},

  {CTRL_RADIO,                      "F&1",            0, &nKey,  1,  4},
  {CTRL_RADIO,                      "F&2",            0, &nKey,  1,  5},
  {CTRL_RADIO,                      "F&3",            0, &nKey,  1,  6},
  {CTRL_RADIO,                      "F&4",            0, &nKey,  1,  7},
  {CTRL_RADIO,                      "F&5",            0, &nKey,  1,  8},
  {CTRL_RADIO,                      "F&6",            0, &nKey,  1,  9},
  {CTRL_RADIO,                      "F&7",            0, &nKey,  1, 10},
  {CTRL_RADIO,                      "F&8",            0, &nKey,  1, 11},
  {CTRL_RADIO,                      "F&9",            0, &nKey,  1, 12},
  {CTRL_RADIO,                      "F1&0",           0, &nKey,  1, 13},

  {CTRL_RADIO,                      "&Up",            0, &nKey, 20,  4},
  {CTRL_RADIO,                      "&Down",          0, &nKey, 20,  5},
  {CTRL_RADIO,                      "&Left",          0, &nKey, 20,  6},
  {CTRL_RADIO,                      "&Right",         0, &nKey, 20,  7},
  {CTRL_RADIO,                      "&Home",          0, &nKey, 20,  8},
  {CTRL_RADIO,                      "&End",           0, &nKey, 20,  9},
  {CTRL_RADIO,                      "PgU&p",          0, &nKey, 20, 10},
  {CTRL_RADIO,                      "PgD&n",          0, &nKey, 20, 11},

  {CTRL_RADIO,                      "&Insert",        0, &nKey, 40,  4},
  {CTRL_RADIO,                      "Dele&te",        0, &nKey, 40,  5},
  {CTRL_RADIO,                      "&Backspace",     0, &nKey, 40,  6},


  {CTRL_LINE,                       "",               0, NULL,   1, 15, 56},

  {CTRL_DEFBUTTON|CTRL_ALLIGN_LEFT, "Key",        ID_OK, NULL,  27, 16,  0,  0, "Generate selected key pressing"},
  {CTRL_BUTTON|CTRL_ALLIGN_LEFT,    "Cancel", ID_CANCEL, NULL,  37, 16},
  {0}
};


int KeyGenDialog::OnClose(int id)
{
  if(id == ID_OK)
  {
    static int Key[] = {
      K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10,
      K_UP, K_DOWN, K_LEFT, K_RIGHT, K_HOME, K_END, K_PAGEUP, K_PAGEDN,
      K_INSERT, K_DELETE, K_BS
    };
    static int Mod[] = {0, K_SHIFT, K_CTRL, K_ALT};

    UpdateVar();
    int code = Key[nKey] | Mod[nMod];

    TPRINT(("gen code=%x\n", code));
    g_pApplication->PutCode(code);
  }

  return 0;
}

#endif