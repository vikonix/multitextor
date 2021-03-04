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
#include "utils/Directory.h"
#include "utils/logger.h"
#include "utils/SymbolType.h"
#include "utfcpp/utf8.h"
#include "App.h"


bool Editor::Clear()
{
    m_buffer.Clear();
    m_undoList.Clear();
    m_wndList.clear();
    m_lexParser.Clear();

    m_curStr = STR_NOTDEFINED;
    m_curChanged = false;
    m_curStrBuff.clear();

    return true;
}

bool Editor::LoadBuff(uint64_t offset, size_t size, std::shared_ptr<std::string> buff)
{
    std::ifstream file{ m_file, std::ios::binary };
    if (!file)
    {
        _assert(0);
        return false;
    }
    
    file.seekg(offset);

    buff->resize(size);
    file.read(buff->data(), size);
    auto read = file.gcount();
    if (size != static_cast<size_t>(read))
    {
        _assert(0);
        return false;
    }

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

    LOG(DEBUG) << __FUNC__ << " path=" << m_file << " size=" << std::hex << fileSize << std::dec;
    time_t start = time(NULL);

    std::ifstream file{m_file, std::ios::binary};
    if (!file)
        return false;

    m_buffer.SetLoadBuffFunc(std::bind(&Editor::LoadBuff, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    Application::getInstance().SetHelpLine("Wait for file loading");

    time_t t1 = time(nullptr);
    size_t percent = 0;
    auto step = fileSize / 100;//1%

    const size_t buffsize = 0x200000;//2MB
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
        return static_cast<size_t>(read);
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
            strBuffData->resize(strOffset + tocopy);
            std::memcpy(strBuffData->data() + strOffset, buff->data() + buffOffset, tocopy);
            strBuff->ReleaseBuff();

            if (strOffset + tocopy < BUFF_SIZE && !file.eof())
                strOffset += tocopy;
            else
            {
                //now fill string offset table
                strBuff->m_fileOffset = fileOffset;

                size_t rest;
                //LOG(DEBUG) << std::hex << "file offset=" << fileOffset << " read=" << read << " strOffset=" << strOffset << " tocopy=" << tocopy << std:: dec;
                [[maybe_unused]]bool rc = FillStrOffset(strBuff, strOffset + tocopy, fileSize == fileOffset + strOffset + tocopy, rest);
                //LOG(DEBUG) << std::hex << "rest=" << rest << std::dec;

                _assert(rc);
                _assert(tocopy > rest);//???

                tocopy -= rest;
                fileOffset += tocopy + strOffset;
                m_buffer.m_totalStrCount += strBuff->GetStrCount();
                strBuff = nullptr;
                strOffset = 0;
            }

            buffOffset += tocopy;
            read -= tocopy;
        }
    }
    _assert(fileSize == fileOffset);
    
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

    size_t begin = 0;
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

            m_lexParser.ScanStr(m_buffer.m_totalStrCount + strBuff->GetStrCount(), {buff + begin, i - begin});
            strBuff->m_strOffsetList.push_back((uint32_t)i + 1);
            begin = i + 1;
            len = 0;
            cut = 0;
        }
        else if (buff[i] == 0xa)
        {
            ++lf;
            m_lexParser.ScanStr(m_buffer.m_totalStrCount + strBuff->GetStrCount(), { buff + begin, i - begin });
            strBuff->m_strOffsetList.push_back((uint32_t)i + 1);
            begin = i + 1;
            len = 0;
            cut = 0;
        }
        else if (buff[i] > 0)
        {
            //check symbol type
            if (GetSymbolType(buff[i]) != symbol_t::alnum)
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

            m_lexParser.ScanStr(m_buffer.m_totalStrCount + strBuff->GetStrCount(), { buff + begin, i - begin });
            strBuff->m_strOffsetList.push_back((uint32_t)i + 1);
            begin = i + 1;
            len = 0;
            cut = 0;
        }
    }

    if (len && last)
    {
        //parse last string in file
        m_lexParser.ScanStr(m_buffer.m_totalStrCount + strBuff->GetStrCount(), { buff + begin, i - begin });
        strBuff->m_strOffsetList.push_back((uint32_t)i);
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

    str->resize(i);
    rest = size - strBuff->GetBuffSize();
    strBuff->ReleaseBuff();

    return true;
}

std::u16string  Editor::GetStr(size_t line, size_t offset, size_t size)
{
    if (line == m_curStr && !m_curStrBuff.empty())
        return m_curStrBuff.substr(offset, size);
    else
        return _GetStr(line, offset, size);
}

std::u16string  Editor::_GetStr(size_t line, size_t offset, size_t size)
{
    if (line >= m_buffer.GetStrCount())
        return std::u16string(size, ' ');

    std::u16string outstr(size, ' ');
    
    auto str = m_buffer.GetStr(line);
    std::u16string wstr = utf8::utf8to16(std::string(str));//???Convert char with cp

    //go from begin of string for right tabulation calculating 
    size_t pos{ 0 };
    for (auto c : wstr)
    {
        if (c > ' ')
        {
            if (pos >= offset)
            {
                outstr[pos - offset] = c;
            }
        }
        if (c == 0x9)//tab
        {
            size_t tabpos = (pos + m_tab) - (pos + m_tab) % m_tab;
            if (m_saveTab || m_showTab)
                while (pos < tabpos)
                {
                    if (pos < offset + size)
                        outstr[pos++ - offset] = 0x9;
                }
            pos = tabpos;
        }
        else if (c == 0xd || c == 0x0a || c == 0x1a)//cr/lf/eof
            break;
        else
            ++pos;

        if (pos >= offset + size)
            break;
    }

    m_buffer.ReleaseBuff();

    return outstr;
}

char Editor::GetAccessInfo()
{
    if (IsChanged())//modified
        return 'M';
    
    auto mode = Directory::GetAccessMode(m_file);
    if (mode == notexists)
        return 'N';//new
    else if (m_ro || mode == readonly)
        return 'R';
    else
        return ' ';
}

bool Editor::GetColor(size_t line, const std::u16string& str, std::vector<color_t>& buff, size_t len)
{
    return m_lexParser.GetColor(line, str, buff, len);
}

bool Editor::RefreshAllWnd(FrameWnd* wnd) const
{
    for (auto w : m_wndList)
    {
        if (w == wnd)
            continue;
        w->Repaint();
    }

    return true;
}

bool Editor::ChangeStr(size_t n, const std::u16string& wstr)
{
    LOG(DEBUG) << "ChangeStr " << n << " total=" << GetStrCount();

    if (n >= GetStrCount())
    {
        bool rc = AddStr(n, wstr);
        return rc;
    }

    std::string str;
    bool rc = ConvertStr(wstr, str);
    rc = m_buffer.ChangeStr(n, str);

    return rc;
}

bool Editor::AddStr(size_t n, const std::u16string& wstr)
{
    bool rc;
    if (n > GetStrCount())
    {
        LOG(DEBUG) << "Fill end of file";

        for (size_t i = GetStrCount(); i < n; ++i)
            rc = _AddStr(i, {});
    }

    rc = _AddStr(n, wstr);
    return rc;
}

bool Editor::_AddStr(size_t n, const std::u16string& wstr)
{
    LOG(DEBUG) << "AddStr n=" << n;

    std::string str;
    bool rc = ConvertStr(wstr, str);
    rc = m_buffer.AddStr(n, str);

    return rc;
}

bool Editor::InvalidateWnd(size_t line, invalidate_t type, pos_t pos, pos_t size) const
{
    for (auto wnd : m_wndList)
        wnd->Invalidate(line, type, pos, size);

    return true;
}

bool Editor::FlushCurStr()
{
    if (m_curChanged)
    {
        ChangeStr(m_curStr, m_curStrBuff);
        m_curChanged = false;
    }

    return true;
}

bool Editor::SetCurStr(size_t line)
{
    if (line != m_curStr)
    {
        if (m_curChanged)
        {
            ChangeStr(m_curStr, m_curStrBuff);
            m_curChanged = false;
        }

        m_curStr = line;
        m_curStrBuff = _GetStr(line, 0, MAX_STRLEN);
    }

    return true;
}

bool Editor::ConvertStr(const std::u16string& str, std::string& buff) const
{
    size_t len = UStrLen(str);

    //convert string
    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] != 0x9)//tab
            buff += (char)str[i];//??? wchar2char(m_nCP, pStr[i]);
        else if (m_saveTab)
        {
            size_t first = i;
            while (str[i] == 0x9)
            {
                ++i;
                if (i % m_tab == 0)
                {
                    buff += 0x9;
                    first = i;
                }
            }

            if (i % m_tab < m_tab)
            {
                //fill as space
                while (first++ < i)
                    buff += ' ';
            }

            --i;
        }
        else
            buff += ' ';
    }

    LOG(DEBUG) << "ConvertStr '" << buff << "'";
    if (m_eol == eol_t::unix_eol)
    {
        buff += 0xa;
    }
    else if (m_eol == eol_t::win_eol)
    {
        buff += 0xd;
        buff += 0xa;
    }
    else
    {
        //apple
        buff += 0xd;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
bool Editor::AddCh(bool save, size_t line, size_t pos, char16_t ch)
{
    if (line >= GetStrCount() && ch == ' ')
        return true;

    return AddSubstr(save, line, pos, {ch});
}

bool Editor::ChangeCh(bool save, size_t line, size_t pos, char16_t ch)
{
    if (line >= GetStrCount() && ch == ' ')
        return true;

    return ChangeSubstr(save, line, pos, {ch});
}

bool Editor::AddSubstr(bool save, size_t line, size_t pos, const std::u16string& substr)
{
    SetCurStr(line);
    m_curStrBuff.insert(pos, substr);
    m_curStrBuff.resize(MAX_STRLEN, ' ');

    if (line >= GetStrCount())
    {
        //editing out of the end of file: add new line 
        AddLine(save, line, m_curStrBuff.substr(0, pos + substr.size()));
        m_curStr = STR_NOTDEFINED;
    }
    else
    {
        m_curChanged = true;
        if (save)
        {
            m_undoList.AddEditCmd(cmd_t::CMD_ADD_SUBSTR, line, pos, 0, substr.size(), substr);
            m_undoList.AddUndoCmd(cmd_t::CMD_DEL_SUBSTR, line, pos, 0, substr.size(), {});
        }

        CorrectTab(save, line, m_curStrBuff);
    }

    invalidate_t inv;
    m_lexParser.ChangeStr(line, m_curStrBuff, inv);
    InvalidateWnd(line, inv);

    return true;
}

bool Editor::ChangeSubstr(bool save, size_t line, size_t pos, const std::u16string& substr)
{
    SetCurStr(line);
    std::u16string prevStr{ m_curStrBuff.substr(pos, substr.size()) };
    m_curStrBuff.replace(pos, substr.size(), substr);
    m_curStrBuff.resize(MAX_STRLEN, ' ');

    if (line >= GetStrCount())
    {
        //editing out of the end of file: add new line
        AddLine(save, line, m_curStrBuff.substr(0, pos + substr.size()));
        m_curStr = STR_NOTDEFINED;
    }
    else
    {
        m_curChanged = true;
        if (save)
        {
            m_undoList.AddEditCmd(cmd_t::CMD_CHANGE_SUBSTR, line, pos, 0, substr.size(), substr);
            m_undoList.AddUndoCmd(cmd_t::CMD_CHANGE_SUBSTR, line, pos, 0, substr.size(), prevStr);
        }

        CorrectTab(save, line, m_curStrBuff);
    }

    invalidate_t inv;
    m_lexParser.ChangeStr(line, m_curStrBuff, inv);
    InvalidateWnd(line, inv);

    return true;
}

bool Editor::CorrectTab(bool save, size_t line, std::u16string& str)
{
    if (!m_saveTab)
        return true;

    LOG(DEBUG) << "CorrectTab save=" << save << " line=" << line;

    size_t len = UStrLen(str);
    std::u16string tabs;

    for(size_t i= 0; i < len; ++i)
    {
        if (str[i] == 0x9)
        {
            size_t first = i;
            while (str[i] == 0x9)
            {
                ++i;
                if (i % m_tab == 0)
                    first = i;
            }

            if (i % m_tab < m_tab)
            {
                //fill as space
                while (first < i)
                {
                    tabs += static_cast<char16_t>(first);
                    str[first++] = ' ';
                }
            }

            --i;
        }
    }

    if (save && !tabs.empty())
    {
        m_undoList.AddEditCmd(cmd_t::CMD_CORRECT_TAB, line, 0, 0, 0, {});
        m_undoList.AddUndoCmd(cmd_t::CMD_RESTORE_TAB, line, 0, 0, tabs.size(), tabs);
    }

    return true;
}

bool Editor::AddLine(bool save, size_t line, const std::u16string& str)
{
    if (m_curStr != STR_NOTDEFINED && line <= m_curStr)
        ++m_curStr;

    int rc;
    size_t count = 0;
    if (line >= GetStrCount())
    {
        LOG(DEBUG) << "Fill end of file";

        count = line - GetStrCount();

        for (size_t i = GetStrCount(); i < line; ++i)
        {
            rc = _AddStr(i, {});
            invalidate_t inv;
            m_lexParser.AddStr(i, {}, inv);
            InvalidateWnd(i, invalidate_t::insert);
        }
    }

    rc = _AddStr(line, str);
    invalidate_t inv = invalidate_t::insert;
    m_lexParser.AddStr(line, str, inv);
    InvalidateWnd(line, inv);

    if (save)
    {
        m_undoList.AddEditCmd(cmd_t::CMD_ADD_LINE, line, 0, 0, str.size(), str);
        m_undoList.AddUndoCmd(cmd_t::CMD_DEL_LINE, line, 0, count, 0, {});
    }

    return rc;
}

bool Editor::DelSubstr(bool save, size_t line, size_t pos, size_t len)
{
    if (line >= GetStrCount())
        return true;

    SetCurStr(line);
    std::u16string prevStr{ m_curStrBuff.substr(pos, len) };
    m_curStrBuff.erase(pos, len);
    m_curStrBuff.resize(MAX_STRLEN, ' ');

    m_curChanged = true;
    invalidate_t inv;
    m_lexParser.ChangeStr(line, m_curStrBuff, inv);
    InvalidateWnd(line, inv);

    if (save)
    {
        m_undoList.AddEditCmd(cmd_t::CMD_DEL_SUBSTR, line, pos, 0, len, {});
        m_undoList.AddUndoCmd(cmd_t::CMD_ADD_SUBSTR, line, pos, 0, len, prevStr);
    }

    CorrectTab(save, line, m_curStrBuff);

    return 0;
}

bool Editor::DelLine(bool save, size_t line, size_t count)
{
    if (line >= GetStrCount())
        return true;

    if (save)
    {
        auto str = GetStr(line);
        size_t len = Editor::UStrLen(str);

        m_undoList.AddEditCmd(cmd_t::CMD_DEL_LINE, line, 0, count, 0, {});
        m_undoList.AddUndoCmd(cmd_t::CMD_ADD_LINE, line, 0, 0, len, str);
    }

    if (line == m_curStr)
    {
        m_curStr = STR_NOTDEFINED;
        m_curChanged = 0;
    }
    else if (line < m_curStr)
        --m_curStr;

    bool rc = m_buffer.DelStr(line);
    invalidate_t inv;
    m_lexParser.DelStr(line, inv);
    InvalidateWnd(line, inv);

    while (count-- > 1)
    {
        rc = m_buffer.DelStr(--line);
        m_lexParser.DelStr(line, inv);
        InvalidateWnd(line, invalidate_t::del);
    }

    return rc;
}

bool Editor::MergeLine(bool save, size_t line, size_t pos, size_t indent)
{
    if (line >= GetStrCount())
        return true;

    //merge current string with prev
    SetCurStr(line);
    if (pos > MAX_STRLEN)
        pos = Editor::UStrLen(m_curStrBuff);

    auto str = GetStr(line + 1);
    size_t len1 = Editor::UStrLen(str);

    if (len1 > indent)
    {
        if (pos + len1 > MAX_STRLEN)
        {
            //too long string
            _assert(0);
            return false;
        }
        m_curStrBuff.insert(pos, str.substr(indent));
        
        m_curChanged = true;
        invalidate_t inv;
        m_lexParser.ChangeStr(line, m_curStrBuff, inv);
        InvalidateWnd(line, inv);
    }

    //del next line
    bool rc = DelLine(0, line + 1);
    InvalidateWnd(line, invalidate_t::del);

    if (save)
    {
        m_undoList.AddEditCmd(cmd_t::CMD_MERGE_LINE, line, pos, indent, 0, {});
        m_undoList.AddUndoCmd(cmd_t::CMD_SPLIT_LINE, line, pos, indent, 0, {});
    }

    CorrectTab(save, line, m_curStrBuff);

    return rc;
}

bool Editor::SplitLine(bool save, size_t line, size_t pos, size_t indent)
{
    if (line >= GetStrCount())
        return true;

    SaveTab(save, line);
    SetCurStr(line);
    
    std::u16string str;
    str.resize(indent, ' ');
    //copy rest of current str to new buff
    str.append(m_curStrBuff.substr(pos));
    //str.resize(MAX_STRLEN, ' '); //???

    //clear end of current str
    m_curStrBuff.replace(pos, m_curStrBuff.size() - pos, m_curStrBuff.size() - pos, ' ');

    m_curChanged = true;
    invalidate_t inv;
    m_lexParser.ChangeStr(line, m_curStrBuff, inv);
    InvalidateWnd(line, inv);

    if (save)
    {
        m_undoList.AddEditCmd(cmd_t::CMD_SPLIT_LINE, line, pos, indent, 0, {});
        m_undoList.AddUndoCmd(cmd_t::CMD_MERGE_LINE, line, pos, indent, 0, {});
    }

    bool rc = AddLine(false, line + 1, str);
    InvalidateWnd(line, invalidate_t::insert);

    return rc;
}

bool Editor::SaveTab(bool save, size_t line)
{
    if (!m_saveTab)
        return true;

    LOG(DEBUG) << "SaveTab line=" << line;

    SetCurStr(line);

    //save tab offset as symbols;
    std::u16string tabs;

    for (size_t i = 0; i < m_curStrBuff.size(); ++i)
        if (m_curStrBuff[i] == 0x9)
            tabs += static_cast<char16_t>(i);

    if (save && !tabs.empty())
    {
        m_undoList.AddEditCmd(cmd_t::CMD_SAVE_TAB, line, 0, 0, 0, {});
        m_undoList.AddUndoCmd(cmd_t::CMD_RESTORE_TAB, line, 0, 0, tabs.size(), tabs);
    }

    return true;
}

bool Editor::ClearSubstr(bool save, size_t line, size_t pos, size_t len)
{
    if (line >= GetStrCount())
        return true;

    SetCurStr(line);

    std::u16string prevstr = m_curStrBuff.substr(pos, len);
    m_curStrBuff.replace(pos, len, len, ' ');

    m_curChanged = true;
    invalidate_t inv;
    m_lexParser.ChangeStr(line, m_curStrBuff, inv);
    InvalidateWnd(line, inv);

    if (save)
    {
        m_undoList.AddEditCmd(cmd_t::CMD_CLEAR_SUBSTR, line, pos, 0, len, {});
        m_undoList.AddUndoCmd(cmd_t::CMD_CHANGE_SUBSTR, line, pos, 0, len, prevstr);
    }

    CorrectTab(save, line, m_curStrBuff);

    return true;
}

bool Editor::AddUndoCommand(const EditCmd& editCmd, const EditCmd& undoCmd)
{
    m_undoList.AddEditCmd(editCmd.command, editCmd.line, editCmd.pos, editCmd.count, editCmd.len, editCmd.str);
    m_undoList.AddUndoCmd(undoCmd.command, undoCmd.line, undoCmd.pos, undoCmd.count, undoCmd.len, undoCmd.str);

    return true;
}

bool Editor::ClearModifyFlag()
{
    //???m_pDObject->CheckAccess(1);
    m_buffer.m_changed = false;
    m_curChanged = false;
    return true;
}

bool Editor::Command(const EditCmd& cmd)
{
    bool rc{};

    switch (cmd.command)
    {
    case cmd_t::CMD_ADD_LINE:
        rc = AddLine(false, cmd.line, cmd.str);
        break;
    case cmd_t::CMD_DEL_LINE:
        rc = DelLine(false, cmd.line, cmd.count);
        break;
    case cmd_t::CMD_MERGE_LINE:
        rc = MergeLine(false, cmd.line, cmd.pos, cmd.count);
        break;
    case cmd_t::CMD_SPLIT_LINE:
        rc = SplitLine(false, cmd.line, cmd.pos, cmd.count);
        break;
    case cmd_t::CMD_ADD_SUBSTR:
        rc = AddSubstr(false, cmd.line, cmd.pos, cmd.str);
        break;
    case cmd_t::CMD_CHANGE_SUBSTR:
        rc = ChangeSubstr(false, cmd.line, cmd.pos, cmd.str);
        break;
    case cmd_t::CMD_CLEAR_SUBSTR:
        rc = ClearSubstr(false, cmd.line, cmd.pos, cmd.len);
        break;
    case cmd_t::CMD_DEL_SUBSTR:
        rc = DelSubstr(false, cmd.line, cmd.pos, cmd.len);
        break;
    case cmd_t::CMD_REPLACE_SUBSTR:
        //???rc = ReplaceSubstr(false, cmd.line, cmd.pos, cmd.count, cmd.str);
        break;
    case cmd_t::CMD_INDENT:
        //???rc = Indent(false, cmd.line, cmd.pos, cmd.len, cmd.count);
        break;
    case cmd_t::CMD_UNDENT:
        //???rc = Undent(false, cmd.line, cmd.pos, cmd.len, cmd.count);
        break;
    case cmd_t::CMD_CORRECT_TAB:
        break;
    case cmd_t::CMD_SAVE_TAB:
        rc = SaveTab(false, cmd.line);
        break;
    case cmd_t::CMD_RESTORE_TAB:
        //???rc = RestoreTab(false, cmd.line, cmd.str);
        break;

    default:
        LOG(ERROR) << "Unknown command " << static_cast<int>(cmd.command);
        break;
    }

    return rc;
}
