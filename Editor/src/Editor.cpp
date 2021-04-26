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
#include "utils/CpConverter.h"
#include "utfcpp/utf8.h"
#include "EditorApp.h"

namespace _Editor
{

constexpr uintmax_t MAX_PARSED_SIZE{ 0x2000000 }; // 32 MB

bool Editor::SetCP(const std::string& cp) 
{
    m_cp = cp.empty() ? "UTF-8" : cp; 
    try
    {
        m_converter = std::make_shared<iconvpp::CpConverter>(m_cp);
    }
    catch (...)
    {
        LOG(ERROR) << __FUNC__;
        _assert(0);
        return false;
    }

    return true;
}

bool Editor::Clear()
{
    m_buffer.Clear();
    m_undoList.Clear();
    m_lexParser.Clear();
    m_curStrBuff.clear();
    m_curStr = STR_NOTDEFINED;
    m_curChanged = false;

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
    _assert(file.good());
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

    auto fileSize{ std::filesystem::file_size(m_file) };
    decltype(fileSize) fileOffset{};
    if (0 == fileSize)
        return true;

    LOG(DEBUG) << __FUNC__ << " path=" << m_file.u8string() << " size=" << std::hex << fileSize << std::dec;
    time_t start{ time(NULL) };

    std::ifstream file{m_file, std::ios::binary};
    if (!file)
        return false;

    m_buffer.SetLoadBuffFunc(std::bind(&Editor::LoadBuff, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    EditorApp::SetHelpLine("Wait for file loading");

    time_t t1{ time(nullptr) };
    size_t percent{};
    auto step{ fileSize / 100 };//1%

    const size_t buffsize{ 0x200000 };//2MB
    auto buff{ std::make_unique<std::array<char, buffsize>>() };
    size_t buffOffset{0};

    auto readFile = [&]() -> size_t {
        buffOffset = 0;
        if (file.eof())
            return 0;
        file.read(buff->data(), buffsize);
        auto read = file.gcount();
        if (0 == read)
            return 0;

        time_t t2{ time(NULL) };
        if (t1 != t2 && step)
        {
            t1 = t2;
            size_t pr{ static_cast<size_t>((fileOffset + read) / step) };
            if (pr != percent)
            {
                percent = pr;
                EditorApp::ShowProgressBar(pr);
            }
        }
        return static_cast<size_t>(read);
    };

    
    if (fileSize > MAX_PARSED_SIZE)
        m_lexParser.EnableParsing(false);

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
            auto strBuffData{ strBuff->GetBuff() };
            if (!strBuffData)
            {
                //no memory
                _assert(0);
                return false;
            }
            size_t tocopy{ std::min(static_cast<size_t>(BUFF_SIZE) - strOffset, read) };
            strBuffData->resize(strOffset + tocopy);
            std::memcpy(strBuffData->data() + strOffset, buff->data() + buffOffset, tocopy);
            strBuff->ReleaseBuff();

            if (strOffset + tocopy < BUFF_SIZE / 2 && !file.eof())
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
                if(tocopy > rest)
                    tocopy -= rest;
                else
                {
                    _assert(0);
                }

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
    
    EditorApp::ShowProgressBar();
    EditorApp::SetHelpLine("Ready", stat_color::grayed);

    LOG(DEBUG) << "load time=" << time(NULL) - start;
    LOG(DEBUG) << "num str=" << m_buffer.m_totalStrCount;

    return true;
}

bool Editor::FillStrOffset(std::shared_ptr<StrBuff<std::string, std::string_view>> strBuff, size_t size, bool last, size_t& rest)
{
    auto str{ strBuff->GetBuff() };
    if (!str)
        return false;

    //1 byte is reserved for 0xA so 0D and 0A EOL will go to same buffers
    //and we not get left empty string
    const size_t maxsize{ !last ? size - 1 : size };
    const size_t maxtab{ 10 };
    size_t cr{};
    size_t crlf{};
    size_t lf{};
    size_t cut{};
    size_t len{};

    size_t begin{};
    size_t i;
    const char* buff {str->c_str()};

    for (i = 0; i < maxsize; ++i)
    {
        unsigned char ch = buff[i];
        ++len;
        if (ch == S_TAB)
        {
            --len;
            //calc len with max tabulation for possible changing in future
            len = (len + maxtab) - (len + maxtab) % maxtab;
        }
        else if (ch == S_CR)
        {
            if (buff[i + 1] == S_LF)
            {
                ++i;
                ++crlf;
            }
            else
                ++cr;

            m_lexParser.ScanStr(m_buffer.m_totalStrCount + strBuff->GetStrCount(), {buff + begin, i - begin}, m_cp);
            strBuff->m_strOffsetList.push_back((uint32_t)i + 1);
            begin = i + 1;
            len = 0;
            cut = 0;
        }
        else if (ch == S_LF)
        {
            ++lf;

            m_lexParser.ScanStr(m_buffer.m_totalStrCount + strBuff->GetStrCount(), { buff + begin, i - begin }, m_cp);
            strBuff->m_strOffsetList.push_back((uint32_t)i + 1);
            begin = i + 1;
            len = 0;
            cut = 0;
        }
        else
        {
            //check symbol type
            if (GetSymbolType(ch) != symbol_t::alnum)
                cut = i;
        }

        if (len >= m_maxStrlen)
        {
            //wrap for long string
            if (buff[i + 1] == S_CR)
            {
                if (buff[i + 2] == S_LF)
                {
                    ++i;
                    ++crlf;
                }
                else
                    ++cr;
                ++i;
            }
            else if (buff[i + 1] == S_LF)
            {
                ++i;
                ++lf;
            }
            else if (cut)
            {
                //cut str by last word
                i = cut;
            }

            m_lexParser.ScanStr(m_buffer.m_totalStrCount + strBuff->GetStrCount(), { buff + begin, i - begin }, m_cp);
            strBuff->m_strOffsetList.push_back((uint32_t)i + 1);
            begin = i + 1;
            len = 0;
            cut = 0;
        }
    }

    if (len && last)
    {
        //parse last string in file
        m_lexParser.ScanStr(m_buffer.m_totalStrCount + strBuff->GetStrCount(), { buff + begin, i - begin }, m_cp);
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
    _assert(i <= BUFF_SIZE);
    rest = size - strBuff->GetBuffSize();
    strBuff->ReleaseBuff();

    return true;
}

bool Editor::FlushCurStr()
{
    bool rc{ true };
    if (m_curChanged)
    {
        rc = ChangeStr(m_curStr, m_curStrBuff);
        m_curChanged = false;
    }

    return rc;
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
        m_curStrBuff = _GetStr(line, 0, m_maxStrlen);
    }

    return true;
}

std::u16string  Editor::GetStr(size_t line, size_t offset, size_t size)
{
    //_assert(offset == 0);
    if (line == m_curStr && !m_curStrBuff.empty())
    {
        if(offset + size <= m_maxStrlen)
            return m_curStrBuff.substr(offset, size);
        else
            return m_curStrBuff.substr(offset);
    }
    else
        return _GetStr(line, offset, size);
}

std::u16string Editor::_GetStr(size_t line, size_t offset, size_t size)
{
    if (line >= m_buffer.GetStrCount())
    {
        if(offset + size <= m_maxStrlen)
            return std::u16string(size - offset, ' ');
        else
            return {};
    }

    auto str{ m_buffer.GetStr(line) };
    std::u16string wstr;
    [[maybe_unused]]bool rc = m_converter->Convert(str, wstr);
    std::u16string outstr;
    if (offset + size <= m_maxStrlen)
        outstr.resize(size, ' ');
    else
        outstr.resize(wstr.size(), ' ');

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
        if (c == S_TAB)
        {
            size_t tabpos = (pos + m_tab) - (pos + m_tab) % m_tab;
            outstr.resize(std::min(outstr.size() + tabpos, size), ' ');

            if (m_saveTab || m_showTab)
                while (pos < tabpos)
                {
                    if (pos < offset + size)
                        outstr[pos++ - offset] = S_TAB;
                }
            pos = tabpos;
        }
        else if (c == S_CR || c == S_LF || c == S_EOF)//cr/lf/eof
            break;
        else
            ++pos;

        if (pos >= offset + size)
            break;
    }

    m_buffer.ReleaseBuff();

    return outstr;
}

std::u16string Editor::GetStrForFind(size_t line, bool checkCase)
{
    if (line >= m_buffer.GetStrCount())
            return {};

    auto str{ m_buffer.GetStr(line) };
    std::u16string wstr;
    [[maybe_unused]] bool rc = m_converter->Convert(str, wstr);
    std::u16string outstr;
    outstr.resize(wstr.size(), ' ');

    //go from begin of string for right tabulation calculating 
    size_t pos{ 0 };
    for (auto c : wstr)
    {
        if (c > ' ')
        {
            if(checkCase)
                outstr[pos] = c;
            else
                outstr[pos] = std::towupper(c);
        }
        if (c == S_TAB)
        {
            size_t tabpos = (pos + m_tab) - (pos + m_tab) % m_tab;
            outstr.resize(outstr.size() + tabpos, ' ');

            pos = tabpos;
        }
        else if (c == S_CR || c == S_LF || c == S_EOF)//cr/lf/eof
            break;
        else
            ++pos;
    }

    m_buffer.ReleaseBuff();

    return outstr;
}


bool Editor::ConvertStr(const std::u16string& str, std::string& buff) const
{
    size_t len = UStrLen(str);

    //convert string
    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] != S_TAB)
        {
            std::string cpStr;
            [[maybe_unused]]bool rc = m_converter->Convert(str[i], cpStr);
            buff += cpStr;
        }
        else if (m_saveTab)
        {
            size_t first = i;
            while (str[i] == S_TAB)
            {
                ++i;
                if (i % m_tab == 0)
                {
                    buff += S_TAB;
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

    //LOG(DEBUG) << "ConvertStr '" << buff << "'";
    if (m_eol == eol_t::unix_eol)
    {
        buff += S_LF;
    }
    else if (m_eol == eol_t::win_eol)
    {
        buff += S_CR;
        buff += S_LF;
    }
    else
    {
        //apple
        buff += S_CR;
    }

    return true;
}

bool Editor::ChangeStr(size_t n, const std::u16string& wstr)
{
    //LOG(DEBUG) << "ChangeStr " << n << " total=" << GetStrCount();

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

char Editor::GetAccessInfo()
{
    if (IsChanged())//modified
        return 'M';
    
    auto mode = Directory::GetAccessMode(m_file);
    if (mode == fileaccess_t::notexists)
        return 'N';//new
    else if (m_ro || mode == fileaccess_t::readonly)
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

bool Editor::AddStr(size_t n, const std::u16string& wstr)
{
    bool rc;
    if (n > GetStrCount())
    {
        //LOG(DEBUG) << "Fill end of file";

        for (size_t i = GetStrCount(); i < n; ++i)
            rc = _AddStr(i, {});
    }

    rc = _AddStr(n, wstr);
    return rc;
}

bool Editor::_AddStr(size_t n, const std::u16string& wstr)
{
    //LOG(DEBUG) << "AddStr n=" << n;

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
    m_curStrBuff.resize(m_maxStrlen, ' ');

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
    m_curStrBuff.resize(m_maxStrlen, ' ');

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

    size_t len{ UStrLen(str) };
    std::u16string tabs;

    for(size_t i = 0; i < len; ++i)
    {
        if (str[i] == S_TAB)
        {
            size_t first = i;
            while (str[i] == S_TAB)
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
    size_t count{};
    if (line >= GetStrCount())
    {
        //LOG(DEBUG) << "Fill end of file";

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
    invalidate_t inv;
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
    m_curStrBuff.resize(m_maxStrlen, ' ');

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

    return true;
}

bool Editor::DelLine(bool save, size_t line, size_t count)
{
    if (line >= GetStrCount())
        return true;

    if (save)
    {
        auto str{ GetStr(line) };
        size_t len{ UStrLen(str) };

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
    if (pos > m_maxStrlen)
        pos = UStrLen(m_curStrBuff);

    auto str{ GetStr(line + 1) };
    size_t len{ UStrLen(str) };

    if (len > indent)
    {
        if (pos + len > m_maxStrlen)
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
    
    std::u16string str(indent, ' ');
    //copy rest of current str to new buff
    str.append(m_curStrBuff.substr(pos));

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
        if (m_curStrBuff[i] == S_TAB)
            tabs += static_cast<char16_t>(i);

    if (save && !tabs.empty())
    {
        m_undoList.AddEditCmd(cmd_t::CMD_SAVE_TAB, line, 0, 0, 0, {});
        m_undoList.AddUndoCmd(cmd_t::CMD_RESTORE_TAB, line, 0, 0, tabs.size(), tabs);
    }

    return true;
}

bool Editor::RestoreTab([[maybe_unused]]bool save, size_t line, const std::u16string& str)
{
    if (!m_saveTab)
        return true;

    LOG(DEBUG) << "RestoreTab line=" << line;

    SetCurStr(line);
    for(auto pos : str)
        m_curStrBuff[pos] = S_TAB;//set tab

    InvalidateWnd(line, invalidate_t::change);
    return true;
}

bool Editor::ClearSubstr(bool save, size_t line, size_t pos, size_t len)
{
    if (line >= GetStrCount())
        return true;

    SetCurStr(line);

    std::u16string prevstr{ m_curStrBuff.substr(pos, len) };
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

bool Editor::ReplaceSubstr(bool save, size_t line, size_t pos, size_t len, const std::u16string& substr)
{
    if (line >= GetStrCount())
        return true;

    SetCurStr(line);

    std::u16string prevStr{ m_curStrBuff.substr(pos, len) };
    m_curStrBuff.replace(pos, len, substr);

    m_curChanged = true;
    invalidate_t inv;
    m_lexParser.ChangeStr(line, m_curStrBuff, inv);
    InvalidateWnd(line, inv);

    if (save)
    {
        m_undoList.AddEditCmd(cmd_t::CMD_REPLACE_SUBSTR, line, pos, len, substr.size(), substr);
        m_undoList.AddUndoCmd(cmd_t::CMD_REPLACE_SUBSTR, line, pos, substr.size(), len, prevStr);
    }

    CorrectTab(save, line, m_curStrBuff);

    return true;
}

bool Editor::Indent(bool save, size_t line, size_t pos, size_t len, size_t n)
{
    if (line >= GetStrCount())
        return true;

    SetCurStr(line);
    
    //count number of spaces before [pos+len]
    size_t count{};
    for (size_t i = 0; i < n; ++i)
        if (m_curStrBuff[pos + len - 1 - i] == ' ')
            ++count;
        else
            break;

    if (count)
    {
        m_curStrBuff.erase(pos + len - 1 - count, count);
        m_curStrBuff.insert(pos, count, ' ');

        m_curChanged = true;
        invalidate_t inv;
        m_lexParser.ChangeStr(line, m_curStrBuff, inv);
        InvalidateWnd(line, inv);

        if (save)
        {
            m_undoList.AddEditCmd(cmd_t::CMD_INDENT, line, pos, count, len, {});
            m_undoList.AddUndoCmd(cmd_t::CMD_UNDENT, line, pos, count, len, {});
        }

        CorrectTab(save, line, m_curStrBuff);
    }

    return true;
}

bool Editor::Undent(bool save, size_t line, size_t pos, size_t len, size_t n)
{
    if (line >= GetStrCount())
        return true;

    SetCurStr(line);

    //count number of spaces after [pos]
    size_t count{};
    for (size_t i = 0; i < n; ++i)
        if (m_curStrBuff[pos + i] == ' ')
            ++count;
        else
            break;

    if (count)
    {
        m_curStrBuff.insert(pos + len - 1 - count, count, ' ');
        m_curStrBuff.erase(pos, count);

        m_curChanged = true;
        invalidate_t inv;
        m_lexParser.ChangeStr(line, m_curStrBuff, inv);
        InvalidateWnd(line, inv);

        if (save)
        {
            m_undoList.AddEditCmd(cmd_t::CMD_UNDENT, line, pos, count, len, {});
            m_undoList.AddUndoCmd(cmd_t::CMD_INDENT, line, pos, count, len, {});
        }

        CorrectTab(save, line, m_curStrBuff);
    }

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
    m_buffer.ClearModifyFlag();
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
        rc = ReplaceSubstr(false, cmd.line, cmd.pos, cmd.count, cmd.str);
        break;
    case cmd_t::CMD_INDENT:
        rc = Indent(false, cmd.line, cmd.pos, cmd.len, cmd.count);
        break;
    case cmd_t::CMD_UNDENT:
        rc = Undent(false, cmd.line, cmd.pos, cmd.len, cmd.count);
        break;
    case cmd_t::CMD_CORRECT_TAB:
        break;
    case cmd_t::CMD_SAVE_TAB:
        rc = SaveTab(false, cmd.line);
        break;
    case cmd_t::CMD_RESTORE_TAB:
        rc = RestoreTab(false, cmd.line, cmd.str);
        break;

    default:
        LOG(ERROR) << "Unknown command " << static_cast<int>(cmd.command);
        break;
    }

    return rc;
}

bool Editor::CheckLexPair(size_t& line, size_t& pos)
{
    auto str{ GetStr(line, 0, m_maxStrlen) };
    size_t y{ line };
    char16_t c{ str[pos] };

    bool rc = m_lexParser.CheckLexPair(str, line, pos);
    if (!rc)
        return false;
    if (y == line)
        return true;

    //matching at another line
    str = GetStr(line, 0, m_maxStrlen);
    return m_lexParser.GetLexPair(str, line, c, pos);
}

bool Editor::Save()
{
    LOG(DEBUG) << "Save " << m_file.u8string();
    time_t start{ time(NULL) };

    bool rc = FlushCurStr();
    rc = BackupFile();

    auto filePath{ m_file };
    std::fstream file{ filePath, std::ios::binary|std::ios::in|std::ios::out };
    if (!file)
        throw std::runtime_error{"open file " + filePath.u8string()};
    _assert(file.good());

    EditorApp::SetHelpLine("Wait for file saving");

    auto Load = [&file](uint64_t offset, size_t size, std::shared_ptr<std::string> buff) {
        file.seekg(offset);
        buff->resize(size);
        file.read(buff->data(), size);
        _assert(file.good());
        auto read = file.gcount();
        if (size != static_cast<size_t>(read))
        {
            _assert(0);
            return false;
        }

        return true;
    };

    time_t t1{ time(nullptr) };
    size_t percent{};
    auto step{ GetSize() / 100 };//1%

    size_t buffOffset{ 0 };
    for (auto buffIt = m_buffer.m_buffList.begin(); buffIt != m_buffer.m_buffList.end(); ++buffIt)
    {
        auto& buffPtr = *buffIt;
        auto buffStr = buffPtr->GetBuff();
        if (!buffStr)
        {
            //error
            _assert(0);
            throw std::runtime_error{ "GetBuffer" };
        }
        if (buffPtr->m_lostData)
        {
            rc = Load(buffPtr->m_fileOffset, buffPtr->GetBuffSize(), buffStr);
            if (!rc)
            {
                //error
                _assert(0);
                throw std::runtime_error{ "LoadBuff" };
            }
            buffPtr->m_lostData = false;
        }

        if(buffPtr->m_strOffsetList.empty())
        {
            buffPtr->ClearModifyFlag();
            buffPtr->ReleaseBuff();
            continue;
        }

        rc = ImproveBuff(buffIt);

        buffPtr->m_fileOffset = buffOffset;
        size_t buffSize = buffPtr->GetBuffSize();

        auto nextIt = buffIt;
        while (++nextIt != m_buffer.m_buffList.end())
        {
            auto& nextBuffPtr = *nextIt;
            //check next buffer for begin offset
            if (nextBuffPtr->m_fileOffset < buffOffset + buffSize)
            {
                //read ahead before write    
                auto nextBuffStr = nextBuffPtr->GetBuff();
                if (!nextBuffStr)
                {
                    //error
                    _assert(0);
                    throw std::runtime_error{ "GetBuffer" };
                }
                if (nextBuffPtr->m_lostData)
                {
                    rc = Load(nextBuffPtr->m_fileOffset, nextBuffPtr->GetBuffSize(), nextBuffStr);
                    if (!rc)
                    {
                        //error
                        _assert(0);
                        throw std::runtime_error{ "LoadBuff" };
                    }
                    nextBuffPtr->m_lostData = false;
                }
            }
            else
                break;
        }

        file.seekp(buffOffset);
        file.write(buffStr->data(), buffSize);
        _assert(file.good());

        buffPtr->ClearModifyFlag();
        buffPtr->ReleaseBuff();
        buffOffset += buffSize;

        time_t t2{ time(NULL) };
        if (t1 != t2 && step)
        {
            t1 = t2;
            size_t pr{ static_cast<size_t>(buffOffset / step) };
            if (pr != percent)
            {
                percent = pr;
                EditorApp::ShowProgressBar(pr);
            }
        }
    }

    file.close();
    auto fsize = std::filesystem::file_size(filePath);
    if (fsize > buffOffset)
    {
        std::filesystem::resize_file(filePath, buffOffset);
    }

    rc = ClearModifyFlag();
    EditorApp::ShowProgressBar();
    EditorApp::SetHelpLine("Ready", stat_color::grayed);

    LOG(DEBUG) << "save time=" << time(nullptr) - start;

    return rc;
}

bool Editor::BackupFile()
{
    //???
    return true;
}

bool Editor::ImproveBuff(std::list<std::shared_ptr<StrBuff<std::string, std::string_view>>>::iterator strIt)
{
    // fix EOL
    // change tabulation
    // remove spaces at EOL
    auto& strBuff = *strIt;
    for (size_t n = 0; n < strBuff->m_strOffsetList.size(); ++n)
    {
        auto str{ strBuff->GetStr(n) };
        std::u16string wstr;
        bool rc = m_converter->Convert(str, wstr);

        std::string outstr;
        outstr.reserve(str.size());

        bool changed{};
        size_t usedSize{};//size of string without of trailing spaces
        size_t wpos{};
        size_t i;
        for (i = 0; i < str.size(); ++i)
        {
            unsigned char c = str[i];
            if (c > ' ')
            {
                outstr += c;
                usedSize = outstr.size();
            }
            else if (c == ' ')
                outstr += ' ';
            else if (c == S_TAB)
            {
                if (m_saveTab)//??? || !rc)
                    outstr += S_TAB;
                else
                {
                    //change tab with space
                    wpos = wstr.find(S_TAB, wpos);
                    if (wpos == std::string::npos)
                    {
                        _assert(0);
                    }
                    else
                    {
                        auto tabs = m_tab - (wpos + m_tab) % m_tab;
                        outstr.append(tabs, ' ');
                        changed = true;
                    }
                }
            }
            else
            {
                if (usedSize != outstr.size())
                {
                    //del all spaces at the end of string
                    outstr.resize(usedSize);
                    changed = true;
                }
                break;
            }
        }
        
        if (m_eol == eol_t::unix_eol)
        {
            outstr += S_LF;
            if (i != str.size() - 1 || str[i] != S_LF)
                changed = true;
        }
        else if (m_eol == eol_t::win_eol)
        {
            outstr += "\r\n";
            if (i != str.size() - 2 || (str[i] != S_CR || str[i + 1] != S_LF))
                changed = true;
        }
        else
        {
            outstr += S_CR;
            if (i != str.size() - 1 || str[i] != S_CR)
                changed = true;
        }

        if (changed)
        {
            _assert(str != outstr);
            changed = false;

            rc = strBuff->ChangeStr(n, outstr);
            if (!rc)
            {
                rc = m_buffer.SplitBuff(strIt, n);
                if (!rc)
                {
                    //error
                    _assert(0);
                    throw std::runtime_error{ "SplitBuff" };
                }
            }
        }
    }

    return true;
}

std::list<FrameWnd*> Editor::GetLinkedWnd(FrameWnd* wnd) const
{
    std::list<FrameWnd*> list;
    for (auto w : m_wndList)
    {
        if (w != wnd)
            list.push_back(w);
    }
    return list;
}

bool Editor::SetName(const std::filesystem::path& file, bool copy)
{
    if(copy)
        std::filesystem::copy(m_file, file, std::filesystem::copy_options::overwrite_existing);
    if (!std::filesystem::exists(file))
    {
        std::ofstream create(file);
    }

    m_file = file;
    return true;
}

bool Editor::SetParseStyle(const std::string& style)
{
    if (style != GetParseStyle())
    {
        LOG(DEBUG) << "Change parse mode to " << style;

        m_lexParser.SetParseStyle(style);
        m_tab = m_lexParser.GetTabSize();
        m_saveTab = m_lexParser.GetSaveTab();

        FlushCurStr();
        for (size_t n = 0; n < GetStrCount(); ++n)
        {
            auto str = m_buffer.GetStr(n);
            m_lexParser.ScanStr(n, str, m_cp);
        }
    }
    
    return true;
}

} //namespace _Editor
