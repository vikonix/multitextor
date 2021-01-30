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
#include "Editor.h"
#include "utils/logger.h"
#include "App.h"


bool Editor::Clear()
{
    m_buffer.Clear();
    m_undoList.Clear();
    m_wndList.clear();
    //m_LexBuff.Clear();

    m_curStr = 0;
    m_curChanged = false;
    m_curStrBuff.clear();

    return true;
}

bool Editor::Load()
{
    try
    {
        if (!std::filesystem::exists(m_file) || !std::filesystem::is_regular_file(m_file))
            return false;
    }
    catch (...)
    {
        return false;
    }

    Clear();

    auto fileSize = std::filesystem::file_size(m_file);
    decltype(fileSize) fileOffset{};
    if (0 == fileSize)
        return true;

    LOG(DEBUG) << __FUNC__ << " path=" << m_file << " size=" << fileSize;
    time_t start = time(NULL);

    std::ifstream file{m_file, std::ios::binary};
    if (!file)
        return false;

    Application::getInstance().SetHelpLine("Wait for file loading");

    time_t t1 = time(nullptr);
    size_t percent = 0;
    auto step = fileSize / 100;

    const size_t buffsize = 0x200000;
    auto buff = std::make_unique<std::array<char, buffsize>>();
    size_t buffOffset{0};

    auto readFile = [&]() -> size_t {
        buffOffset = 0;
        if (file.eof())
            return 0;
        file.read(buff->data(), buffsize);
        auto read = file.gcount();
        if (0 == read)
            return 0;

        time_t t2 = time(NULL);
        if (t1 != t2 && step)
        {
            t1 = t2;
            size_t pr = (size_t)((fileOffset + read) / step);
            if (pr != percent)
            {
                percent = pr;
                Application::getInstance().ShowProgressBar(pr);
            }
        }
        return read;
    };

    std::shared_ptr<StrBuff<std::string, std::string_view>> strBuff;
    size_t strOffset{};
    size_t read;
    while (0 != (read = readFile()))
    {
        while (read)
        {
            if (!strBuff)
            {
                strBuff = m_buffer.GetNewBuff();
                strOffset = 0;
            }
            auto strBuffData = strBuff->GetBuff();
            if (!strBuffData)
            {
                //no memory
                _assert(0);
                return false;
            }
            size_t tocopy = std::min((size_t)BUFF_SIZE - strOffset, read);
            std::memcpy(strBuffData->data() + strOffset, buff->data() + buffOffset, tocopy);
            strBuff->ReleaseBuff();

            if (strOffset + tocopy < BUFF_SIZE && !file.eof())
                strOffset += tocopy;
            else
            {
                //now fill string offset table
                strBuff->m_fileOffset = fileOffset;
                size_t rest;
                [[maybe_unused]]bool rc = FillStrOffset(strBuff, tocopy, fileSize == fileOffset + tocopy, rest);
                _assert(rc);

                strOffset = 0;
                m_buffer.m_totalStrCount += strBuff->m_strCount;
                strBuff = nullptr;
            }

            buffOffset += tocopy;
            fileOffset += tocopy;
            read -= tocopy;
        }
    }

    LOG(DEBUG) << "loadtime=" << time(NULL) - start;
    LOG(DEBUG) << "num str=" << m_buffer.m_totalStrCount;

    Application::getInstance().ShowProgressBar();
    Application::getInstance().SetHelpLine("Ready", stat_color::grayed);

    return true;
}

bool Editor::FillStrOffset(std::shared_ptr<StrBuff<std::string, std::string_view>> strBuff, size_t size, bool last, size_t& rest)
{
    auto str = strBuff->GetBuff();
    if (!str)
        return false;

    //1 byte is reserved for 0xA so 0D and 0A EOL will go to same buffers
    //and we not get left empty string
    const size_t maxsize = !last ? size - 1 : size;
    const size_t maxtab = 10;
    size_t cr = 0;
    size_t crlf = 0;
    size_t lf = 0;
    size_t cut = 0;

    size_t len = 0;
    size_t i;
    const char* buff = str->c_str();

    for (i = 0; i < maxsize; ++i)
    {
        ++len;
        if (buff[i] == 0x9)//TAB
        {
            --len;
            //calc len with max tabulation for possible changing in future
            len = (len + maxtab) - (len + maxtab) % maxtab;
        }
        else if (buff[i] == 0xd)//CR
        {
            if (buff[i + 1] == 0xa)//LF
            {
                ++i;
                ++crlf;
            }
            else
                ++cr;

            //m_LexBuff.ScanStr(m_nStrCount + pStr->m_StrCount, pCurStr, i - (pCurStr - pBuff));
            //pCurStr = pBuff + i + 1;
            strBuff->m_strOffsets[++strBuff->m_strCount] = (uint32_t)(i + 1);
            len = 0;
            cut = 0;
        }
        else if (buff[i] == 0xa)
        {
            ++lf;
            //m_LexBuff.ScanStr(m_nStrCount + pStr->m_StrCount, pCurStr, i - (pCurStr - pBuff));
            //pCurStr = pBuff + i + 1;
            strBuff->m_strOffsets[++strBuff->m_strCount] = (uint32_t)(i + 1);
            len = 0;
            cut = 0;
        }
        else if (buff[i] > 0)
        {
            //check symbol type
            //???if (GetSymbolType(buff[i]) != 6)
            if(buff[i] == ' ')
                cut = i;
        }

        if (len >= m_maxStrlen)
        {
            //wrap for long string
            if (buff[i + 1] == 0xd)
            {
                if (buff[i + 2] == 0xa)
                {
                    ++i;
                    ++crlf;
                }
                else
                    ++cr;
                ++i;
            }
            else if (buff[i + 1] == 0xa)
            {
                ++i;
                ++lf;
            }
            else if (cut)
            {
                //cut str by last word
                i = cut;
            }

            //m_LexBuff.ScanStr(m_nStrCount + pStr->m_StrCount, pCurStr, i - (pCurStr - pBuff));
            //pCurStr = pBuff + i + 1;
            strBuff->m_strOffsets[++strBuff->m_strCount] = (uint32_t)(i + 1);
            len = 0;
            cut = 0;
        }

        if (strBuff->m_strCount == STR_NUM)
            break;
    }

    if (len && last)
    {
        //parse last string in file
        //m_LexBuff.ScanStr(m_nStrCount + pStr->m_StrCount, pCurStr, i - (pCurStr - pBuff));
        strBuff->m_strOffsets[++strBuff->m_strCount] = (uint32_t)i;
    }

    if (0 == strBuff->m_fileOffset)
    {
        LOG(DEBUG) << "cr=" << cr << " lf=" << lf << " crlf=" << crlf;
        auto m = std::max({lf, crlf, cr});
        if(m == lf)
            m_eol = eol_t::unix_eol; //unix
        else if(m == crlf)
            m_eol = eol_t::win_eol; //windows
        else
            m_eol = eol_t::mac_eol; //apple
    }

    rest = size - strBuff->m_strOffsets[strBuff->m_strCount];
    strBuff->ReleaseBuff();

    return true;
}

#if 0


int TextBuff::SetShowTab(int show)
{
  m_fShowTab = show;
  return m_LexBuff.SetShowTab(show);
}


int TextBuff::LinkWnd(FWnd* pWnd)
{
  for(int i = 0; i < WND_NUM; ++i)
    if(!m_pLinkedWnd[i])
    {
      m_pLinkedWnd[i] = pWnd;
      return 0;
    }

  return -1;
}


int TextBuff::UnlinkWnd(FWnd* pWnd)
{
  int n = 0;
  for(int i = 0; i < WND_NUM; ++i)
    if(m_pLinkedWnd[i] == pWnd)
      m_pLinkedWnd[i] = NULL;
    else if(m_pLinkedWnd[i])
      ++n;

  return n;
}


FWnd* TextBuff::GetLinkWnd(FWnd* pWnd)
{
  for(int i = 0; i < WND_NUM; ++i)
    if(m_pLinkedWnd[i] && m_pLinkedWnd[i] != pWnd)
      return m_pLinkedWnd[i];

  return NULL;
}

/*
int TextBuff::WndLinkCount()
{
  int n = 0;
  for(int i = 0; i < WND_NUM; ++i)
    if(m_pLinkedWnd[i])
      ++n;

  return n;
}
*/

int TextBuff::InvalidateWnd(size_t nline, int type, short pos, short size)
{
  for(int i = 0; i < WND_NUM; ++i)
    if(m_pLinkedWnd[i])
    {
      m_pLinkedWnd[i]->Invalidate(nline, type, pos, size);
    }

  return 0;
}


int TextBuff::RefreshAllWnd(FWnd* pWnd)
{
  for(int i = 0; i < WND_NUM; ++i)
    if(m_pLinkedWnd[i] &&  m_pLinkedWnd[i] != pWnd)
      m_pLinkedWnd[i]->Repaint();

  return 0;
}




char TextBuff::GetAccessInfo()
{
  if(m_fChanged || m_fCurChanged)
    return 'M';
  else if(m_pDObject->GetMode() == 3)
    return ' ';
  else if(m_pDObject->GetMode() == 1)
    return 'R';
  else
    return 'N';
}


int TextBuff::ClearModifyFlag()
{
  //TPRINT(("ClearModifyFlag\n"));
  m_pDObject->CheckAccess(1);
  m_fChanged    = 0;
  m_fCurChanged = 0;
  return 0;
}


int TextBuff::CheckAccess()
{
  return m_pDObject->CheckAccess(1);
}



int TextBuff::GetStr(size_t n, size_t offset, wchar* pBuff, size_t len)
{
  size_t i;
  //fill dest buffer with space
  for(i = 0; i < len; ++i)
    pBuff[i] = ' ';
  pBuff[i] = 0;

  if(n >= m_nStrCount)
    return 0;

  size_t size;
  char* pStr = MStrBuff::GetStr(n, &size);
  int s = 0;

  //всегда идем с начала строки чтобы учесть все табуляции
  i = 0;
  for(int j = 0; j < offset + len && i < size;)
  {
    unsigned char c = pStr[i++];
    if(c == ' ')
      ++j;
    else if(c > ' ')
    {
      if(j >= offset)
      {
        wchar wc = char2wchar(m_nCP, c);
        pBuff[j - offset] = wc;
      }
      s = ++j;
    }
    else if(c == 0x9)//tab
    {
      //TPRINT(("tab size=%d from=%d\n", m_nTab, j));
      int t = (j + m_nTab) - (j + m_nTab) % m_nTab;
      if(m_fSaveTab || m_fShowTab)
        while(j < t)
          pBuff[j++ - offset] = 0x9;
      else
        while(j < t)
          pBuff[j++ - offset] = ' ';
    }
    else if(c == 0xd || c == 0x0a || c == 0x1a)//cr/lf/eof
      break;
    else
    {
#if 0
      break;
#else
      if(j >= offset)
      {
        pBuff[j - offset] = ' ';
      }
      ++j;
#endif
    }
  }

  MStrBuff::ReleaseBuff();

  return s - offset;//real size
}


int TextBuff::ConvertStr(wchar* pStr, char* pOutStr, int len)
{
  //looking for end of string
  int s = -1;
  int i;
  for(i = 0; i < len; ++i)
    if(!pStr[i])
      break;
    else if(pStr[i] > ' ')
      s = i;
  len = s + 1;

  int l = 0;
  //convert string
  for(i = 0; i < len; ++i)
  {
    if(pStr[i] != 0x9)
      pOutStr[l++] = wchar2char(m_nCP, pStr[i]);
    else if(m_fSaveTab)
    {
      int first = i;
      while(pStr[i] == 0x9)
      {
        ++i;
        if(i % m_nTab == 0)
        {
          pOutStr[l++] = 0x9;
          first = i;
        }
      }

      if(i % m_nTab < m_nTab)
      {
        //fill as space
        while(first++ < i)
          pOutStr[l++] = ' ';
      }

      --i;
    }
    else
      pOutStr[l++] = ' ';
  }

  if(!m_nCrLf)
  {
    //unix
    pOutStr[l++] = 0xa;
  }
  else if(m_nCrLf == 1)
  {
    //dos
    pOutStr[l++] = 0xd;
    pOutStr[l++] = 0xa;
  }
  else
  {
    //apple
    pOutStr[l++] = 0xd;
  }

  pOutStr[l] = 0;

  //TPRINT(("ConvertStr %s", pOutStr));

  return l;
}


int TextBuff::AddStr(size_t str, wchar* pStr, int len)
{
  int rc;
  if(str > m_nStrCount)
  {
    TPRINT(("Fill end of file\n"));

    wchar new_str[] = {' ', 0};

    for(size_t i = m_nStrCount; i < str; ++i)
      rc = _AddStr(i, new_str, 1);
  }

  rc = _AddStr(str, pStr, len);

  return rc;
}


int TextBuff::_AddStr(size_t str, wchar* pStr, int len)
{
  //TPRINT(("AddStr n=%d l=%d\n", str, len));

  if(len == -1)
    len = MAX_STRLEN;
  char* pOutStr = new char[(len + 3) * 2];
  if(!ASSERT(pOutStr!= NULL))
  {
    return 0;
  }

  int l = ConvertStr(pStr, pOutStr, len);
  int rc = MStrBuff::AddStr(str, pOutStr, l);

  delete pOutStr;

  return rc;
}


int TextBuff::ChangeStr(size_t str, wchar* pStr, int len)
{
  size_t n = str;
  //TPRINT(("ChangeStr %d all=%d\n", n, m_nStrCount));
  int rc;

  if(n >= m_nStrCount)
  {
    rc = AddStr(str, pStr, len);
    return rc;
  }

  if(len == -1)
    len = MAX_STRLEN;
  char* pOutStr = new char[(len + 3) * 2];
  if(!ASSERT(pOutStr != NULL))
  {
    return 0;
  }

  int l = ConvertStr(pStr, pOutStr, len);

  rc = MStrBuff::ChangeStr(str, pOutStr, l);
  delete pOutStr;

  return 0;
}


int TextBuff::Save()
{
  TPRINT(("Save\n"));

  if(m_fCurChanged)
  {
    int rc = ChangeStr(m_CurStr, m_sCurStrBuff);
    if(rc < 0)
    {
      //что то не так, возможно нет свободных буферов
      return rc;
    }
    m_fCurChanged = 0;
  }

  if(g_nBackupCount)
  {
    //make backup
    long long size = m_pDObject->GetSize();
    if(size >= 32 && size < MAX_BACKUP_SIZE)
    {
      char path[MAX_PATH];
#ifdef _WIN32
      SDir::MkDir(g_sBackupPath, "bak", path);
#else
      char path1[MAX_PATH];
      SDir::MkDir(g_sBackupPath, "m.bak", path1);
      SDir::MkDir(path1, SDir::GetUserName(), path);
#endif

      DirList dir;
      dir.ControlFilesCount(path, "*.bak", g_nBackupCount - 1);

      char buff[32];
      sprintf_s(buff, sizeof(buff), ".%04x.bak", (unsigned int)(time(NULL) & 0xffff));
      char outbuff[MAX_PATH + 1];
      int rc = m_pDObject->CopyObject(path, buff, outbuff);
      assert(rc == 0);
#ifdef __APPLE__
      rc = chown(outbuff, 99, 99);//only for root
#endif
      (void)rc;
    }
  }

  //save
  int rc = m_pDObject->OpenObject(1);
  if(rc < 0)
    return rc;

  long long curpos = 0;
  size_t str    = 0;
  StrBuff* pSBuff = m_pSBuff;

  while(pSBuff)
  {
    char* pBuff = pSBuff->GetBuff();
    if(!ASSERT(pBuff != NULL))
    {
      rc = -1;
      break;
    }

    size_t size = pSBuff->m_StrCount ? pSBuff->m_pStrOffset[pSBuff->m_StrCount - 1] : 0;
    //TPRINT(("n=%d size=%d\n", pSBuff->m_StrCount, size));
    if(!size)
    {
      pSBuff->ClearModifyFlag();
      pSBuff->ReleaseBuff();
      pSBuff = pSBuff->m_next;
      continue;
    }

    if(pSBuff->m_fLostData)
    {
      size_t r;
      rc = m_pDObject->Read(pBuff, pSBuff->m_nBuffBeginOffset, size, &r);
      if(rc < 0)
        break;
      pSBuff->m_fLostData = 0;
    }

    size_t w;
    pSBuff->m_nBuffBeginOffset = curpos;
    pBuff = ConvertBuff(pSBuff, &size);

    if(pSBuff->m_next)
    {
      //check next buffer for begin offset
      StrBuff* pNextBuff = pSBuff->m_next;
      //TPRINT(("Check next for begin=%d end=%d\n", curpos, curpos + size));

      while(pNextBuff)
      {
        if(pNextBuff->m_nBuffBeginOffset < curpos + size)
        {
          //TPRINT((">>buff begin=%d end=%d\n", pNextBuff->m_nBuffBeginOffset, pNextBuff->m_nStrOffset[pNextBuff->m_StrCount - 1]));

          char* buff = pNextBuff->GetBuff();
          if(!ASSERT(buff != NULL))
          {
            rc = -1;
            break;
          }

          if(pNextBuff->m_fLostData)
          {
            rc = m_pDObject->Read(buff, pNextBuff->m_nBuffBeginOffset,
              pNextBuff->m_pStrOffset[pNextBuff->m_StrCount - 1]);
            if(rc < 0)
              break;
            pNextBuff->m_fLostData = 0;
          }
        }
        else
          break;

        pNextBuff = pNextBuff->m_next;
      }
    }

    rc = m_pDObject->Write(pBuff, curpos, size, &w);
    if(rc < 0)
      break;

    str += pSBuff->m_StrCount;
    pSBuff->ClearModifyFlag();
    pSBuff->ReleaseBuff();
    curpos += w;

    pSBuff = pSBuff->m_next;
  }

  m_pDObject->SetEndOfFile(curpos);
  m_pDObject->CloseObject();
  rc = ClearModifyFlag();

  return 0;
}


int TextBuff::SaveAs(char* pName)
{
  int rc = m_pDObject->CopyObject(pName);

  //if(rc) //???
  //  return rc;

  if(m_pDObject)
    if(m_pDObject->DelLock() == 0)
      delete m_pDObject;

  m_pDObject = new FileObject(pName);
  m_pDObject->AddLock();

  return rc;
}


char* TextBuff::ConvertBuff(StrBuff* pBuff, size_t* pSize)
{
  //нужно добавить или поменять конец строки
  //заменить табуляции
  //убрать лишние пробелы в конце строки
  int rc;
  for(int i = 0; i < pBuff->m_StrCount; ++i)
  {
    size_t begin = (!i) ? 0 : pBuff->m_pStrOffset[i - 1];
    size_t end   = pBuff->m_pStrOffset[i];
    size_t len   = end - begin;

    char* pStr = pBuff->m_pBuff + begin;
    char buff[MAX_STRLEN + 3];
    memset(buff, ' ', sizeof(buff));
    int k = 0, j;
    int fChanged = 0;
    int LastC = 0;//index of last not space symbol
    for(j = 0; j < len; ++j)
    {
      unsigned char c = pStr[j];
      if(c > ' ')
      {
        buff[k++] = c;
        LastC = k;
      }
      else if(c == ' ')
        ++k;
      else if(c == 0x9)//tab
      {
        //TPRINT(("tab size=%d from=%d\n", m_nTab, j));
        if(m_fSaveTab)
          buff[k++] = c;
        else
        {
          k = (k + m_nTab) - (k + m_nTab) % m_nTab;
          fChanged = 1;
        }
      }
      else
      {
        if(LastC != k)
        {
          //del all spaces at the end of string
          fChanged = 1;
          k = LastC;
        }
        break;
      }
    }

    if(m_nCrLf == 0)
    {
      if(j == len || pStr[j] != 0xa)
        fChanged  = 1;
      buff[k++] = 0xa;
      buff[k]   = 0;
    }
    else if(m_nCrLf == 1)
    {
      if(j == len || (pStr[j] != 0xd || pStr[j + 1] != 0xa))
        fChanged  = 1;
      buff[k++] = 0xd;
      buff[k++] = 0xa;
      buff[k]   = 0;
    }

    if(fChanged)
    {
      fChanged = 0;

      //TPRINT(("ConvertStr %d l=%d:%s", i, k, buff));
      TPRINT(("ConvertStr %d l=%d\n", i, k));
      //TDUMP((buff, k + 1, "str"));
      rc = pBuff->ChangeStr(i, buff, k);
      if(rc <= 0)
      {
        TPRINT(("ERROR %d\n", rc));
        //??? split buffer
      }
    }
  }

  *pSize = pBuff->m_pStrOffset[pBuff->m_StrCount - 1];

  return pBuff->m_pBuff;
}


/////////////////////////////////////////////////////////////////////////////
int TextBuff::GetColor(size_t nline, wchar* pStr, color_t* pBuff, size_t len)
{
  return m_LexBuff.GetColor(nline, pStr, pBuff, len);
}


int TextBuff::CheckLexPair(size_t* pLine, int* pX)
{
  size_t y = *pLine;
  wchar* pStr = GetStr(y);
  wchar  c = pStr[*pX];

  int rc = m_LexBuff.CheckLexPair(pStr, pLine, pX);
  if(!rc)
    return 0;
  if(y == *pLine)
    return 1;

  //совпадение на другой строке
  pStr = GetStr(*pLine);
  return m_LexBuff.GetLexPair(pStr, *pLine, c, pX);
}


int TextBuff::GetFuncList(List* pList, int* pLine)
{
  int count = 0;
  int cur = -1;
  for(int i = 0; i < m_nStrCount; ++i)
  {
    wchar* pStr = GetStr(i);
    int nline = m_LexBuff.CheckFunc(i, pStr);
    if(nline >= 0)
    {
      ++count;
      pStr = GetStr(nline);
      int len = GetStrLen(pStr);
      int j;
      char str[MAX_STRLEN + 1];
      for(j = 0; pStr[j] && j < len; ++j)
        str[j] = wchar2char(g_textCP, pStr[j]);
      str[j] = 0;

      char fstr[MAX_STRLEN + 1 + 10];
      sprintf_s(fstr, sizeof(fstr), "%4d %s", nline + 1, str);

      if(cur == -1 && *pLine < nline)
      {
        cur = pList->GetStrCount();
        if(cur)
          --cur;
        //TPRINT(("cur=%d line=%d cur=%d\n", *pLine, line, cur));
      }
      pList->AppendStr(fstr, strlen(fstr) + 1);
      //TPRINT(("Func %s\n", fstr));
    }
  }

  //assert(cur >= 0);
  *pLine = cur;
  return count;
}


/////////////////////////////////////////////////////////////////////////////
wchar* TextBuff::GetStr(size_t nline, size_t offset, size_t size)
{
  wchar* pStr = m_sStrBuff;
  if(nline == m_CurStr)
  {
    memcpy(pStr, m_sCurStrBuff + offset, size * 2);
    pStr[size]    = 0;
    m_nStrBuffLen = -1;
  }
  else
  {
    m_nStrBuffLen = GetStr(nline, offset, pStr, size);
  }

  return pStr;
}


int TextBuff::SetCurStr(size_t nline)
{
  int rc = 0;
  if(nline != m_CurStr)
  {
    if(m_fCurChanged)
    {
      ChangeStr(m_CurStr, m_sCurStrBuff);
      m_fCurChanged = 0;
    }

    m_CurStr = nline;
    if(nline >= 0)
      rc = GetStr(nline, 0, m_sCurStrBuff, MAX_STRLEN);
  }
  return rc;
}


int TextBuff::GetStrLen(wchar* pStr)
{
  if(pStr == m_sStrBuff && m_nStrBuffLen >= 0)
    return m_nStrBuffLen;

  size_t len = _wcslen(pStr);//MAX_STRLEN - 1;
  while(len > 0)
  {
    if(pStr[--len] > ' ')
    {
      ++len;
      break;
    }
  }

  return len;
}


/////////////////////////////////////////////////////////////////////////////
int TextBuff::CorrectTab(int fSave, size_t nline, wchar* pStr)
{
  if(!m_fSaveTab)
    return 0;

  TPRINT(("CorrectTab save=%d line=%d\n", fSave, nline));

  wchar tab[MAX_STRLEN];
  int   tpos = 0;

  for(int i = 0; pStr[i] && i < MAX_STRLEN; ++i)
  {
    if(pStr[i] == 0x9)
    {
      int first = i;
      while(pStr[i] == 0x9)
      {
        ++i;
        if(i % m_nTab == 0)
          first = i;
      }

      if(i % m_nTab < m_nTab)
      {
        //fill as space
        while(first < i)
        {
          tab[tpos++] = first;
          pStr[first++] = ' ';
        }
      }

      --i;
    }
  }

  if(fSave && tpos)
  {
    m_UndoList.AddEditCmd(CMD_CORRECT_TAB, nline, 0, 0, 0,    NULL, m_pRem);
    m_UndoList.AddUndoCmd(CMD_RESTORE_TAB, nline, 0, 0, tpos, tab,  m_pRem);
  }

  return 0;
}


int TextBuff::SaveTab(int fSave, size_t nline)
{
  if(!m_fSaveTab)
    return 0;

  TPRINT(("SaveTab line=%d\n", nline));

  SetCurStr(nline);

  wchar tab[MAX_STRLEN];
  int   tpos = 0;

  for(int i = 0; m_sCurStrBuff[i] && i < MAX_STRLEN; ++i)
    if(m_sCurStrBuff[i] == 0x9)
      tab[tpos++] = i;

  if(fSave && tpos)
  {
    m_UndoList.AddEditCmd(CMD_SAVE_TAB,    nline, 0, 0, 0,    NULL, m_pRem);
    m_UndoList.AddUndoCmd(CMD_RESTORE_TAB, nline, 0, 0, tpos, tab,  m_pRem);
  }

  return 0;
}


int TextBuff::RestoreTab(int /*fSave*/, size_t nline, wchar* pStr, size_t len)
{
  if(!m_fSaveTab)
    return 0;

  TPRINT(("RestoreTab line=%d len=%d\n", nline, len));

  SetCurStr(nline);
  for(int i = 0; i < len; ++i)
    m_sCurStrBuff[pStr[i]] = 0x9;

  InvalidateWnd(nline, 1);
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
int TextBuff::AddLine(int fSave, size_t nline, wchar* pStr, int len)
{
  //len <= MAX_STRLEN
  if(m_CurStr != -1 && nline <= m_CurStr)
    ++m_CurStr;

  int rc;
  size_t count = 0;
  if(nline >= m_nStrCount)
  {
    TPRINT(("Fill end of file\n"));

    count = nline - m_nStrCount;

    wchar new_str[] = {' ', 0};
    for(size_t i = m_nStrCount; i < nline; ++i)
    {
      rc = _AddStr(i, new_str, 1);
      m_LexBuff.AddStr(i, new_str);
      InvalidateWnd(i, 3);
    }
  }

  if(len == -1)
    len = GetStrLen(pStr);

  rc = _AddStr(nline, pStr, len);
  int inv = 3;
  m_LexBuff.AddStr(nline, pStr, &inv);
  InvalidateWnd(nline, inv);

  if(fSave)
  {
    m_UndoList.AddEditCmd(CMD_ADD_LINE, nline, 0, 0,     len, pStr, m_pRem);
    m_UndoList.AddUndoCmd(CMD_DEL_LINE, nline, 0, count,   0, NULL, m_pRem);
  }

  //???CorrectTab(fSave, nline, pStr);

  return rc;
}


int TextBuff::DelLine(int fSave, size_t nline, size_t count)
{
  if(nline >= m_nStrCount)
    return 0;

  if(fSave)
  {
    wchar* pStr = GetStr(nline);
    int len = GetStrLen(pStr);

    m_UndoList.AddEditCmd(CMD_DEL_LINE, nline, 0, count,   0, NULL, m_pRem);
    m_UndoList.AddUndoCmd(CMD_ADD_LINE, nline, 0, 0,     len, pStr, m_pRem);
  }

  if(nline == m_CurStr)
  {
    m_CurStr     = -1;
    m_fCurChanged = 0;
  }
  else if (nline < m_CurStr)
    --m_CurStr;

  int rc = DelStr(nline);
  int inv = 2;
  m_LexBuff.DelStr(nline, &inv);
  InvalidateWnd(nline, inv);

  while(count-- > 1)
  {
    rc = DelStr(--nline);
    m_LexBuff.DelStr(nline, &inv);
    InvalidateWnd(nline, 2);
  }

  return rc;
}


int TextBuff::MergeLine(int fSave, size_t nline, int pos, int unindent)
{
  if(nline >= m_nStrCount)
    return 0;

  //склеить текущую строку со следующей
  SetCurStr(nline);
  if(pos == -1)
    pos = GetStrLen(m_sCurStrBuff);

  wchar* pStr = GetStr(nline + 1, 0, MAX_STRLEN);
  int len1 = GetStrLen(pStr);

  if(len1 > unindent)
  {
    pStr += unindent;
    len1 -= unindent;

    if(pos + len1 > MAX_STRLEN)
    {
      //строка получилась длинее чем MAX_STRLEN ???
      //len1 = MAX_STRLEN - pos;
      assert(0);
      return -1;
    }
    memcpy(m_sCurStrBuff + pos, pStr, len1 * 2);

    m_fCurChanged = 1;
    int inv = 1;
    m_LexBuff.ChangeStr(nline, m_sCurStrBuff, &inv);
    //InvalidateWnd(nline, 1, pos, len1);
    InvalidateWnd(nline, inv);
  }

  //del next line
  int rc = DelLine(0, nline + 1);
  InvalidateWnd(nline, 2);

  if(fSave)
  {
    m_UndoList.AddEditCmd(CMD_MERGE_LINE, nline, pos, unindent, 0, NULL, m_pRem);
    m_UndoList.AddUndoCmd(CMD_SPLIT_LINE, nline, pos, unindent, 0, NULL, m_pRem);
  }

  CorrectTab(fSave, nline, m_sCurStrBuff);

  return rc;
}


int TextBuff::SplitLine(int fSave, size_t nline, int pos, int indent)
{
  if(nline >= m_nStrCount)
    return 0;

  SaveTab(fSave, nline);

  SetCurStr(nline);
  wchar str[MAX_STRLEN + 1];//??? m_sStrBuff

  int i, j = 0;
  for(; j < indent; ++j)
    str[j] = ' ';
  //copy rest of str to new buff
  for(i = pos; i < MAX_STRLEN; ++i, ++j)
    str[j] = m_sCurStrBuff[i];
  //fill end of new str
  for(; j < MAX_STRLEN; ++j)
    str[j] = ' ';
  str[MAX_STRLEN] = 0;

  //fill end of str
  for(i = pos; i < MAX_STRLEN; ++i)
    m_sCurStrBuff[i] = ' ';

  m_fCurChanged = 1;
  int inv = 1;
  m_LexBuff.ChangeStr(nline, m_sCurStrBuff, &inv);
  //InvalidateWnd(nline, 1, pos);
  InvalidateWnd(nline, inv);

  if(fSave)
  {
    m_UndoList.AddEditCmd(CMD_SPLIT_LINE, nline, pos, indent, 0, NULL, m_pRem);
    m_UndoList.AddUndoCmd(CMD_MERGE_LINE, nline, pos, indent, 0, NULL, m_pRem);
  }

  int rc = AddLine(0, nline + 1, str);
  InvalidateWnd(nline, 3);

  return rc;
}


int TextBuff::AddSubstr(int fSave, size_t nline, int pos, wchar* pSubstr, size_t len)
{
  SetCurStr(nline);

  //check for len > MAX_STRLEN ???
  if(pos + len > MAX_STRLEN)
    len = MAX_STRLEN - pos;

  //make hole
  memmove(m_sCurStrBuff + pos + len, m_sCurStrBuff + pos, (MAX_STRLEN - pos - len) * 2);
  //copy substr
  for(int i = 0; i < len; ++i)
    m_sCurStrBuff[pos + i] = pSubstr[i];

  if(nline >= m_nStrCount)
  {
    //мы редактируем за концом файла сохраняем изменение в буфере увеличивая размер файла
    wchar buff[MAX_STRLEN + 1];
    memset(buff, 0, sizeof(buff));
    memcpy(buff, m_sCurStrBuff, (pos + len + 1) * 2);

    m_CurStr = -1;
    AddLine(fSave, nline, buff, pos + len);
  }
  else
  {
    m_fCurChanged = 1;
    if(fSave)
    {
      m_UndoList.AddEditCmd(CMD_ADD_SUBSTR, nline, pos, 0, len, pSubstr, m_pRem);
      m_UndoList.AddUndoCmd(CMD_DEL_SUBSTR, nline, pos, 0, len, NULL, m_pRem);
    }

    CorrectTab(fSave, nline, m_sCurStrBuff);
  }

  //InvalidateWnd(nline, 1, pos);
  int inv = 1;
  m_LexBuff.ChangeStr(nline, m_sCurStrBuff, &inv);
  InvalidateWnd(nline, inv);

  return 0;
}


int TextBuff::ChangeSubstr(int fSave, size_t nline, int pos, wchar* pSubstr, size_t len)
{
  SetCurStr(nline);
  if(pos + len > MAX_STRLEN)
    len = MAX_STRLEN - pos;

  wchar prevstr[MAX_STRLEN];
  //copy substr
  for(int i = 0; i < len; ++i)
  {
    prevstr[i] = m_sCurStrBuff[pos + i];
    m_sCurStrBuff[pos + i] = pSubstr[i];
  }

  if(nline >= m_nStrCount)
  {
    //мы редактируем за концом файла сохраняем изменение в буфере увеличивая размер файла
    wchar buff[MAX_STRLEN + 1];
    memcpy(buff, m_sCurStrBuff, (pos + len + 1) * 2);

    m_CurStr = -1;
    AddLine(fSave, nline, buff, pos + len);
  }
  else
  {
    m_fCurChanged = 1;
    if(fSave)
    {
      m_UndoList.AddEditCmd(CMD_CHANGE_SUBSTR, nline, pos, 0, len, pSubstr, m_pRem);
      m_UndoList.AddUndoCmd(CMD_CHANGE_SUBSTR, nline, pos, 0, len, prevstr, m_pRem);
    }

    CorrectTab(fSave, nline, m_sCurStrBuff);
  }

  //InvalidateWnd(nline, 1, pos, len);
  int inv = 1;
  m_LexBuff.ChangeStr(nline, m_sCurStrBuff, &inv);
  InvalidateWnd(nline, inv);

  return 0;
}


int TextBuff::ClearSubstr(int fSave, size_t nline, int pos, size_t len)
{
  if(nline >= m_nStrCount)
    return 0;

  SetCurStr(nline);
  if(pos + len > MAX_STRLEN)
    len = MAX_STRLEN - pos;

  wchar prevstr[MAX_STRLEN];
  //clear substr
  for(int i = 0; i < len; ++i)
  {
    prevstr[i] = m_sCurStrBuff[pos + i];
    m_sCurStrBuff[pos + i] = ' ';
  }

  m_fCurChanged = 1;
  //InvalidateWnd(line, 1, pos, len);
  int inv = 1;
  m_LexBuff.ChangeStr(nline, m_sCurStrBuff, &inv);
  InvalidateWnd(nline, inv);

  if(fSave)
  {
    m_UndoList.AddEditCmd(CMD_CLEAR_SUBSTR, nline, pos, 0, len, NULL, m_pRem);
    m_UndoList.AddUndoCmd(CMD_CHANGE_SUBSTR, nline, pos, 0, len, prevstr, m_pRem);
  }

  CorrectTab(fSave, nline, m_sCurStrBuff);

  return 0;
}


int TextBuff::DelSubstr(int fSave, size_t nline, int pos, size_t len)
{
  if(nline >= m_nStrCount)
    return 0;

  SetCurStr(nline);
  if(pos + len > MAX_STRLEN)
    len = MAX_STRLEN - pos;

  wchar prevstr[MAX_STRLEN];
  for(int j = 0; j < len; ++j)
    prevstr[j] = m_sCurStrBuff[pos + j];

  //del substr
  memmove(m_sCurStrBuff + pos, m_sCurStrBuff + pos + len, (MAX_STRLEN - pos - len) * 2);
  //clear end
  for(int i = 0; i < len; ++i)
    m_sCurStrBuff[MAX_STRLEN - 1 - i] = ' ';

  m_fCurChanged = 1;
  //InvalidateWnd(line, 1, pos);
  int inv = 1;
  m_LexBuff.ChangeStr(nline, m_sCurStrBuff, &inv);
  InvalidateWnd(nline, inv);

  if(fSave)
  {
    m_UndoList.AddEditCmd(CMD_DEL_SUBSTR, nline, pos, 0, len, NULL, m_pRem);
    m_UndoList.AddUndoCmd(CMD_ADD_SUBSTR, nline, pos, 0, len, prevstr, m_pRem);
  }

  CorrectTab(fSave, nline, m_sCurStrBuff);

  return 0;
}


int TextBuff::ReplaceSubstr(int fSave, size_t nline, int pos, int len, wchar* pSubstr, int size)
{
  if(nline >= m_nStrCount)
    return 0;

  SetCurStr(nline);

  //check for len > MAX_STRLEN ???
  if(pos + len > MAX_STRLEN)
    len = MAX_STRLEN - pos;

  wchar prevstr[MAX_STRLEN];
  for(int j = 0; j < len; ++j)
    prevstr[j] = m_sCurStrBuff[pos + j];

  //TPRINT(("Replace len=%d size=%d\n", len, size));
  if(len > size)
  {
    int n = len - size;
    //del substr
    memmove(m_sCurStrBuff + pos, m_sCurStrBuff + pos + n, (MAX_STRLEN - pos - n) * 2);
    //clear end
    for(int i = 0; i < n; ++i)
      m_sCurStrBuff[MAX_STRLEN - 1 - i] = ' ';
  }
  else if(len < size)
  {
    int n = size - len;
    //make hole
    memmove(m_sCurStrBuff + pos + n, m_sCurStrBuff + pos, (MAX_STRLEN - pos - n) * 2);
  }

  //change substr
  for(int i = 0; i < size; ++i)
    m_sCurStrBuff[pos + i] = pSubstr[i];

  m_fCurChanged = 1;
  //InvalidateWnd(nline, 1, pos);
  int inv = 1;
  m_LexBuff.ChangeStr(nline, m_sCurStrBuff, &inv);
  InvalidateWnd(nline, inv);

  if(fSave)
  {
    m_UndoList.AddEditCmd(CMD_REPLACE_SUBSTR, nline, pos, len, size, pSubstr, m_pRem);
    m_UndoList.AddUndoCmd(CMD_REPLACE_SUBSTR, nline, pos, size, len, prevstr, m_pRem);
  }

  CorrectTab(fSave, nline, m_sCurStrBuff);

  return 0;
}


int TextBuff::Indent(int fSave, size_t nline, int pos, int len, size_t n)
{
  if(nline >= m_nStrCount)
    return 0;

  SetCurStr(nline);
  if(pos + len > MAX_STRLEN)
    len = MAX_STRLEN - pos;

  int count = 0;
  for(int i = 0; i < n; ++i)
    if(m_sCurStrBuff[pos + len - 1 - i] == ' ')
      ++count;
    else
      break;

  if(count)
  {
    //TPRINT(("indent l=%d %d\n", StrY, size));
    memmove(m_sCurStrBuff + pos + count, m_sCurStrBuff + pos, (len - count) * 2);
    for(int i = 0; i < count; ++i)
      m_sCurStrBuff[pos + i] = ' ';

    m_fCurChanged = 1;
    //InvalidateWnd(nline, 1, pos, len);
    int inv = 1;
    m_LexBuff.ChangeStr(nline, m_sCurStrBuff, &inv);
    InvalidateWnd(nline, inv);

    if(fSave)
    {
      m_UndoList.AddEditCmd(CMD_INDENT, nline, pos, count, len, NULL, m_pRem);
      m_UndoList.AddUndoCmd(CMD_UNDENT, nline, pos, count, len, NULL, m_pRem);
    }

    CorrectTab(fSave, nline, m_sCurStrBuff);
  }

  return 0;
}


int TextBuff::Undent(int fSave, size_t nline, int pos, int len, size_t n)
{
  if(nline >= m_nStrCount)
    return 0;

  SetCurStr(nline);
  if(pos + len > MAX_STRLEN)
    len = MAX_STRLEN - pos;

  int count = 0;
  for(int i = 0; i < n; ++i)
    if(m_sCurStrBuff[pos + i] == ' ')
      ++count;
    else
      break;

  if(count)
  {
    //TPRINT(("indent l=%d %d\n", StrY, size));
    memmove(m_sCurStrBuff + pos, m_sCurStrBuff + pos + count, (len - count) * 2);
    for(int i = 0; i < count; ++i)
      m_sCurStrBuff[pos + len - 1 - i] = ' ';

    m_fCurChanged = 1;
    //InvalidateWnd(nline, 1, pos, len);
    int inv = 1;
    m_LexBuff.ChangeStr(nline, m_sCurStrBuff, &inv);
    InvalidateWnd(nline, inv);

    if(fSave)
    {
      m_UndoList.AddEditCmd(CMD_UNDENT, nline, pos, count, len, NULL, m_pRem);
      m_UndoList.AddUndoCmd(CMD_INDENT, nline, pos, count, len, NULL, m_pRem);
    }

    CorrectTab(fSave, nline, m_sCurStrBuff);
  }

  return 0;
}


int TextBuff::AddCh(int fSave, size_t nline, int pos, wchar ch)
{
  if(nline >= m_nStrCount && ch == ' ')
    return 0;

  return AddSubstr(fSave, nline, pos, &ch, 1);
}


int TextBuff::ChangeCh(int fSave, size_t nline, int pos, wchar ch)
{
  if(nline >= m_nStrCount && ch == ' ')
    return 0;

  return ChangeSubstr(fSave, nline, pos, &ch, 1);
}


/////////////////////////////////////////////////////////////////////////////
int TextBuff::AddUndoCommand(EditCmd* pEditCmd, EditCmd* pUndoCmd)
{
  m_UndoList.AddEditCmd(
    pEditCmd->command, pEditCmd->nline, pEditCmd->pos,
    pEditCmd->count, pEditCmd->len, pEditCmd->ch, m_pRem);

  m_UndoList.AddUndoCmd(
    pUndoCmd->command, pUndoCmd->nline, pUndoCmd->pos,
    pUndoCmd->count, pUndoCmd->len, pUndoCmd->ch, m_pRem);

  return 0;
}


EditCmd* TextBuff::GetUndo()
{
  EditCmd* pCmd = m_UndoList.GetUndoCmd();
  return pCmd;
}


EditCmd* TextBuff::GetRedo()
{
  EditCmd* pCmd = m_UndoList.GetEditCmd();
  return pCmd;
}


EditCmd* TextBuff::PeekUndo()
{
  EditCmd* pCmd = m_UndoList.PeekUndoCmd();
  return pCmd;
}


EditCmd* TextBuff::PeekRedo()
{
  EditCmd* pCmd = m_UndoList.PeekEditCmd();
  return pCmd;
}


int TextBuff::Command(EditCmd* pCmd)
{
/*
  TPRINT(("Command i=%d cmd=%d l=%d p=%d c=%d l=%d rem=%s ch=%x\n",
    pCmd->index, pCmd->command, pCmd->nline, pCmd->pos, pCmd->count, pCmd->len, pCmd->remark, pCmd->ch[0]
  ));
//*/
  int rc = -1;
  switch(pCmd->command)
  {
  case CMD_ADD_LINE:
    rc = AddLine(0, pCmd->nline, pCmd->ch, pCmd->len);
    break;
  case CMD_DEL_LINE:
    rc = DelLine(0, pCmd->nline, pCmd->count);
    break;
  case CMD_MERGE_LINE:
    rc = MergeLine(0, pCmd->nline, pCmd->pos, pCmd->count);
    break;
  case CMD_SPLIT_LINE:
    rc = SplitLine(0, pCmd->nline, pCmd->pos, pCmd->count);
    break;
  case CMD_ADD_SUBSTR:
    rc = AddSubstr(0, pCmd->nline, pCmd->pos, pCmd->ch, pCmd->len);
    break;
  case CMD_CHANGE_SUBSTR:
    rc = ChangeSubstr(0, pCmd->nline, pCmd->pos, pCmd->ch, pCmd->len);
    break;
  case CMD_CLEAR_SUBSTR:
    rc = ClearSubstr(0, pCmd->nline, pCmd->pos, pCmd->len);
    break;
  case CMD_DEL_SUBSTR:
    rc = DelSubstr(0, pCmd->nline, pCmd->pos, pCmd->len);
    break;
  case CMD_REPLACE_SUBSTR:
    rc = ReplaceSubstr(0, pCmd->nline, pCmd->pos, pCmd->count, pCmd->ch, pCmd->len);
    break;
  case CMD_INDENT:
    rc = Indent(0, pCmd->nline, pCmd->pos, pCmd->len, pCmd->count);
    break;
  case CMD_UNDENT:
    rc = Undent(0, pCmd->nline, pCmd->pos, pCmd->len, pCmd->count);
    break;
  case CMD_CORRECT_TAB:
    break;
  case CMD_SAVE_TAB:
    rc = SaveTab(0, pCmd->nline);
    break;
  case CMD_RESTORE_TAB:
    rc = RestoreTab(0, pCmd->nline, pCmd->ch, pCmd->len);
    break;

  default:
    TPRINT(("Unknown command %d\n", pCmd->command));
    break;
  }
  return rc;
}

#endif