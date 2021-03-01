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
#include "LexParser.h"
#include "ColorMap.h"
#include "Editor.h"
#include "utils/logger.h"
#include "utfcpp/utf8.h"


std::list<LexConfig> LexParser::s_lexConfig =
{
    {
        //plain text
        "Text",     //name
        "",         //file mask
        "`'\"",     //delimiters
        "",         //name symbols
        {},         //special
        {},         //line
        {},         //open
        {},         //close
        0,          //toggled
        0,          //recursive
        0,          //not case
        0,          //Save Tab
        8,          //tab size
        {}          //key words
    },
    {
        //C/C++
        "C++",    //name
        "*.c;*.cc;*.cpp;*.h;*.hpp",//file mask
        "[]{}();",  //delimiters
        "$",        //name symbols
        {},         //special
        {"//"},     //line
        {"/*"},     //open
        {"*/"},     //close
        0,          //toggled
        0,          //recursive
        0,          //not case
        0,          //Save Tab
        4,          //tab size
        //key words
        {
            "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break", "case", "catch",
            "char", "char8_t", "char16_t", "char32_t", "class", "compl", "const", "constexpr", "const_cast", "continue",
            "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explisit", "export", "extern",
            "false", "float", "for", "final", "finally", "friend", "goto", "if", "inline", "int", "long", "longlong", "mutable", "namespace", "new",
            "noexept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "override", "private", "protected", "public",
            "register", "reinterpret_cast", "restrict", "return", "short", "signed", "sizeof", "static", "static_assert",
            "static_cast", "struct", "switch", "template", "this", "throw", "true", "try", "typedef", "typeid", "typename",
            "union", "unsigned", "use", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xoe_or",
            "far", "near", "huge",
            //preprocessor
            "elif", "endif", "ifdef", "ifndef", "define", "defined", "undef", "include", "line", "error", "pragma", "once", "NULL"
        }
    }
};

//////////////////////////////////////////////////////////////////////////////
bool LexParser::SetParseStyle(int cp, const std::string& style)
{
    m_cp = cp;

    m_scan = false;
    m_parseStyle.clear();
    m_lexPosition.clear();

    m_special.clear();
    m_lineComment.clear();
    m_openComment.clear();
    m_closeComment.clear();

    for (auto& cfg : s_lexConfig)
    {
        if (cfg.langName == style)
        {
            m_parseStyle = cfg.langName;

            for (char16_t i = 0; i < lexTabSize; ++i)
                m_lexTab[i] = static_cast<lex_t>(GetSymbolType(i));
            for (int d : cfg.delimiters)
                m_lexTab[d] = lex_t::DELIMITER;
            for (int s : cfg.nameSymbols)
                m_lexTab[s] = lex_t::SYMBOL;

            for (auto special : cfg.special)
                m_special.insert(special);
            for (auto lineComment : cfg.lineComment)
                m_lineComment.insert(lineComment);
            for (auto openComment : cfg.openComment)
                m_openComment.insert(openComment);
            for (auto closeComment : cfg.closeComment)
                m_closeComment.insert(closeComment);

            m_keyWords = cfg.keyWords;

            m_recursiveComment = cfg.recursiveComment;
            m_toggledComment = cfg.toggledComment;
            m_notCase = cfg.notCase;
            m_saveTab = cfg.saveTab;
            m_tabSize = cfg.tabSize;

            m_showTab = false;

            if (!m_openComment.empty() || !m_closeComment.empty())
                m_scan = true;

            return true;
        }
    }
    
    return false;
}

//////////////////////////////////////////////////////////////////////////////
bool LexParser::ScanStr(size_t line, std::string_view str)
{
    if (!m_scan)
        return true;

    //LOG(DEBUG) << "ScanStr(" << line << ") '" << std::string(str) << "'";

    std::string lexstr;
    bool rc = LexicalParse(str, lexstr);

    if (rc && !lexstr.empty())
    {
        //LOG(DEBUG) << "  collected lex types=" << lexstr;

        m_lexPosition.emplace(line, lexstr);
    }

    return rc;
}

bool LexParser::GetColor(size_t line, const std::u16string& wstr, std::vector<color_t>& color, size_t len)
{
    size_t strLen = Editor::UStrLen(wstr);
    if (strLen == 0)
    {
        color.resize(len, ColorWindow);
        return true;
    }

    CheckForConcatenatedLine(line);
    CheckForOpenComments(line);

    std::string str = utf8::utf16to8(wstr.substr(0, strLen));
    std::string lexstr;
    LexicalParse(str, lexstr, true);

    //LOG(DEBUG) << "GetColor(" << line << ") '" << str << "' len=" << len << " cut=" << m_cutLine << " strSymbol=" << m_stringSymbol;
    //LOG(DEBUG) << "  color='" << lex << "'";

    for (size_t i = 0; i < lexstr.size(); ++i)
        switch (lexstr[i])
        {
        case '3'://back slash
        case '5'://operator
        case 'K'://key word
            color.push_back(ColorWindowLKeyW);
            break;
        case 'R'://rem
            if (str[i] == '{' && i < len - 1 && str[i + 1] == '$')//???
            {
                //pascal preprocessor
                color.push_back(ColorWindowLDelim);
                color.push_back(ColorWindowLDelim);
                ++i;
                for (++i; i < len && str[i] != '}'; ++i)
                    color.push_back(ColorWindowLRem);
                if (str[i] == '}')
                    color.push_back(ColorWindowLDelim);
                else
                    color.push_back(ColorWindowLRem);
            }
            else
                color.push_back(ColorWindowLRem);
            break;
        case 'N'://number
        case '2'://str
            color.push_back(ColorWindowLConst);
            break;
        case '8'://delim
            color.push_back(ColorWindowLDelim);
            break;
        default:
            //case ' '://space
            //case '1'://space
            //case '3'://back slash
            //case '6'://symbol
            if (str[i] != 0x9 || !m_showTab)
                color.push_back(ColorWindow);
            else
                color.push_back(ColorWindowTab);//tab
            break;
        }

    color.resize(len, ColorWindow);
    return true;
}

//////////////////////////////////////////////////////////////////////////////
bool LexParser::CheckForConcatenatedLine(size_t line)
{
    bool cutLine{};
    if (line)
    {
        auto prevIt = m_lexPosition.find(line - 1);
        if (prevIt != m_lexPosition.end())
        {
            //check for concatenated string with '\\' 
            auto[prevLine, prevLex] = *prevIt;
            if (prevLex.back() == '\\')
            {
                cutLine = m_cutLine = true;
                m_stringSymbol = prevLex[prevLex.size() - 2];
            }
        }
    }
    if (!cutLine)
    {
        m_cutLine = false;
        m_stringSymbol = 0;
    }
    
    return true;
}

bool LexParser::LexicalParse(std::string_view str, std::string& buff, bool color)
{
    if (!m_cutLine)
        m_stringSymbol = 0;

    m_cutLine = false;
    m_commentLine = false;

    char skipComment{};
    size_t skipCount{};

    size_t begin{};
    size_t end{};
    lex_t type;
    while ((type = LexicalScan(str, begin, end)) != lex_t::END)
    {
        //LOG(DEBUG) << "  lexem[" << end - begin + 1 << "]='" << std::string(str.substr(begin, end - begin + 1)) << "'";

        if (color)
        {
            if (m_commentLine || m_commentOpen)
                buff.resize(begin, 'R');
            else
                buff.resize(begin, ' ');
        }

        size_t offset = 0;
        if (type == lex_t::STRING
         || type == lex_t::DELIMITER
         || type == lex_t::SYMBOL
         || type == lex_t::OPERATOR
            )
        {
            //for symbols need full matching
            //for string check only some beginning symbols
            //for delimiters check first symbol as comment and if it matches then check some beginning symbols
            if (type == lex_t::OPERATOR)
            {
                if (str[end] == '$')
                {
                    //LOG(DEBUG) << "    Check skip comment begin=" << begin << " end=" << end << " '" << std::string(str.substr(begin));
                    //bash var test
                    if (str[end + 1] == '(')
                        skipComment = ')';
                    else if (str[end + 1] == '{')
                        skipComment = '}';
                }
            }

            lex_t comment{};
            size_t e;
            if (ScanSpecial(str.substr(begin), e))
            {
                end = begin + e;
                offset = end;
            }
            else if ((comment = ScanCommentFromBegin(str.substr(begin), e)) != lex_t::END)
            {
                //LOG(DEBUG) << "    Comment from begin";
                if (type != lex_t::SYMBOL || e == end - begin)
                {
                    //LOG(DEBUG) << "    comment 1 t=" << static_cast<int>(comment);
                    if (!m_commentOpen && comment == lex_t::COMMENT_LINE && !skipComment)
                    {
                        //LOG(DEBUG) << "    COMMENT_LINE";
                        if (!m_commentLine)
                            m_commentLine = true;
                        else if (m_toggledComment)
                            m_commentLine = false;
                    }

                    if (!m_commentLine && comment == lex_t::COMMENT_OPEN)
                    {
                        //LOG(DEBUG) << "    COMMENT_OPEN";
                        ++m_commentOpen;
                    }

                    if (comment == lex_t::COMMENT_CLOSE)
                    {
                        //LOG(DEBUG) << "    COMMENT_CLOSE";
                        if (m_commentLine && m_commentOpen != 0)
                            m_commentLine = false;
                        if (m_recursiveComment)
                            --m_commentOpen;
                        else
                            m_commentOpen = 0;
                    }

                    end = begin + e;
                    offset = end;

                    if (color)
                    {
                        buff.resize(end, 'R');
                    }
                    else
                    {
                        if (!m_commentLine && comment == lex_t::COMMENT_OPEN)
                        {
                            if (buff.empty() || buff.back() != 'O')
                                buff += 'O';
                        }
                        else if (comment == lex_t::COMMENT_CLOSE)
                            buff += 'C';
                    }
                }
            }
            else
            {
                if (!m_commentLine)
                {
                    char c = str[begin];
                    if (!color)
                    {
                        if (type == lex_t::DELIMITER)
                        {
                            if (c == '('
                             || c == '{'
                             || c == '['
                             || c == '<')
                                buff += c;
                            else if (c == ')')
                            {
                                if (buff.empty() || buff.back() != '(')
                                    buff += ')';
                                else
                                    buff.pop_back();
                            }
                            else if (c == '}')
                            {
                                if (buff.empty() || buff.back() != '{')
                                    buff += '}';
                                else
                                    buff.pop_back();
                            }
                            else if (c == ']')
                            {
                                if (buff.empty() || buff.back() != '[')
                                    buff += ']';
                                else
                                    buff.pop_back();
                            }
                            else if (c == '>')
                            {
                                if (buff.empty() || buff.back() != '<')
                                    buff += '>';
                                else
                                    buff.pop_back();
                            }
                        }
                        else if (type == lex_t::STRING)
                        {
                            if (str[end] == '\\')
                            {
                                buff += (char)m_stringSymbol;
                                buff += '\\';
                            }
                        }
                    }

                    if (c == '(')
                    {
                        if (skipComment == ')')
                            ++skipCount;
                    }
                    else if (c == '{')
                    {
                        if (skipComment == '}')
                            ++skipCount;
                    }
                    else if (c == ')')
                    {
                        if (skipComment == ')' && --skipCount == 0)
                            skipComment = 0;
                    }
                    else if (c == '}')
                    {
                        if (skipComment == '}' && --skipCount == 0)
                            skipComment = 0;
                    }
                }

                if (color && !m_commentLine && !m_commentOpen)
                    if (type == lex_t::SYMBOL)
                    {
                        char fill = '0' + static_cast<char>(type);
                        if (IsNumeric(str.substr(begin)))
                            fill = 'N';
                        else
                        {
                            if (IsKeyWord(str.substr(begin, end - begin + 1)))
                                fill = 'K';
                        }

                        buff.resize(end + 1, fill);
                    }
            }
        }

        if (type == lex_t::OPERATOR)
        {
            lex_t comment;
            size_t r1, r2;
            while (begin + offset <= end && (comment = ScanComment(str.substr(begin + offset, end - begin - offset), r1, r2)) != lex_t::END)
            {
                //LOG(DEBUG) << "    Comment inside";
                if (offset && begin + offset + r2 == end)
                {
                    //LOG(DEBUG) << "    OUT comment r1=" << r1 << " r2=" << r2 << " =" << std::string(str.substr(begin + offset));
                    break;
                }

                if (color && !m_commentLine && !m_commentOpen && r1)
                    buff.resize(begin + offset + r1, '0' + static_cast<char>(type));

                if (!m_commentOpen && comment == lex_t::COMMENT_LINE)
                {
                    //LOG(DEBUG) << "    COMMENT_LINE1";
                    m_commentLine = true;
                }

                if (!m_commentLine && comment == lex_t::COMMENT_OPEN)
                {
                    //LOG(DEBUG) << "    COMMENT_OPEN1";
                    ++m_commentOpen;
                }

                if (comment == lex_t::COMMENT_CLOSE)
                {
                    //LOG(DEBUG) << "    COMMENT_CLOSE1";
                    if (m_commentOpen && m_commentLine)
                        m_commentLine = false;
                    if (m_recursiveComment)
                    {
                        if(m_commentOpen)
                            --m_commentOpen;
                    }
                    else
                        m_commentOpen = 0;
                }

                offset += r2 + 1;

                if (color)
                {
                    buff.resize(begin + offset, 'R');
                }
                else
                {
                    if (!m_commentLine && comment == lex_t::COMMENT_OPEN)
                    {
                        if (buff.empty() || buff.back() != 'O')
                            buff += 'O';
                    }
                    else if (comment == lex_t::COMMENT_CLOSE)
                        buff += 'C';
                }
            }
        }

        if (color)
        {
            if (m_commentLine || m_commentOpen)
                buff.resize(end + 1, 'R');
            else
                buff.resize(end + 1, '0' + static_cast<char>(type));
        }

        if (end == str.size())
            break;

        begin = end + 1;
    }

    return true;
}

lex_t LexParser::SymbolType(int c)
{
    if (c & 0x80)
        return lex_t::OTHER;//??? GetSymbolType (char2wchar(m_nCP, c));
    else
        return m_lexTab[c];
}

lex_t LexParser::LexicalScan(std::string_view str, size_t& begin, size_t& end)
{
    lex_t type{};
    size_t strSize = str.size();

    //skip leading space
    while (begin < strSize && (type = SymbolType(str[begin])) == lex_t::SPACE)
        ++begin;

    end = begin;

    if (m_stringSymbol && type != lex_t::END)
        //string continues from prev line
        type = lex_t::STRING;

    switch (type)
    {
    case lex_t::BACKSLASH:
        m_cutLine = true;
    case lex_t::END:
    case lex_t::DELIMITER:
        //along symbol
        break;

    case lex_t::OPERATOR:
    case lex_t::SYMBOL:
    case lex_t::OTHER:
    case lex_t::ERROR:
    case lex_t::SPACE:
    default:
        //connected symbols
        while (end < strSize - 1 && type == SymbolType(str[end + 1]))
            ++end;
        break;

    case lex_t::STRING: //" ' `
        {
            if (m_commentLine || m_commentOpen)
                break;
            if (!m_stringSymbol)
            {
                //begin of the string
                m_stringSymbol = str[begin];
                ++end;
            }

            lex_t t;
            while (end < strSize && (t = SymbolType(str[end])) != lex_t::END)
            {
                if (t == lex_t::STRING && str[end] == m_stringSymbol)
                {
                    //end of string
                    m_stringSymbol = 0;
                    break;
                }
                else if (t == lex_t::BACKSLASH)
                {
                    if (end < strSize - 1 && SymbolType(str[end + 1]) > lex_t::SPACE)
                    {
                        //esc sequence
                        ++end;
                    }
                    else
                    {
                        m_cutLine = true;
                        break;
                    }
                }
                ++end;
            }
            break;
        }
    }

    return type;
}

bool LexParser::ScanSpecial(std::string_view lexem, size_t& end)
{
    for (auto& special : m_special)
    {
        if (lexem.find(special) == 0)
        {
            end = special.size();
            return true;
        }
    }
    
    return false;
}

bool LexParser::IsNumeric(std::string_view lexem)
{
    if ((lexem[0] >= '0' && lexem[0] <= '9') || lexem[0] == '#')
        return true;
    else
        return false;
}

bool LexParser::IsKeyWord(std::string_view lexem)
{
    if (m_keyWords.find(std::string(lexem)) != m_keyWords.end())
        return true;
    else
        return false;
}

//line comment shields opened and hides closed
//first opened comment shields other opened comments
//closed comment always only one
lex_t LexParser::ScanCommentFromBegin(std::string_view lexem, size_t& end)
{
    if (!m_commentLine || m_toggledComment)
        //check for line comment
        for (auto& comment : m_lineComment)
        {
            if (lexem.find(comment) == 0)
            {
                end = comment.size() - 1;
                return lex_t::COMMENT_LINE;
            }
        }

    if (!m_commentLine)
        //check for open comment
        for (auto& comment : m_openComment)
        {
            if (lexem.find(comment) == 0)
            {
                end = comment.size() - 1;
                return lex_t::COMMENT_OPEN;
            }
        }

    //check for close comment
    for (auto& comment : m_closeComment)
    {
        if (lexem.find(comment) == 0)
        {
            end = comment.size() - 1;
            return lex_t::COMMENT_CLOSE;
        }
    }

    return lex_t::END;
}

lex_t LexParser::ScanComment(std::string_view lexem, size_t& begin, size_t& end)
{
    size_t line{};
    size_t open{};
    size_t close{};
    size_t lineSize{};
    size_t openSize{};
    size_t closeSize{};

    if (!m_commentLine)
    {
        //check for line comment
        for (auto& comment : m_lineComment)
        {
            if ((line = lexem.find(comment)) != std::string::npos)
            {
                lineSize = comment.size();
                break;
            }
        }

        //check for open comment
        for (auto& comment : m_openComment)
        {
            if ((open = lexem.find(comment)) != std::string::npos)
            {
                openSize = comment.size();
                break;
            }
        }
    }

    //check for close comment
    for (auto& comment : m_closeComment)
    {
        if ((close = lexem.find(comment)) != std::string::npos)
        {
            closeSize = comment.size();
            break;
        }
    }

    lex_t type{};
    if (lineSize && (!openSize || line < open) && (!closeSize || line < close))
    {
        type = lex_t::COMMENT_LINE;
        begin = line;
        end = begin + lineSize;
    }
    else if (openSize && (!lineSize || open < line) && (!closeSize || open < close))
    {
        type = lex_t::COMMENT_OPEN;
        begin = open;
        end = begin + openSize;
    }
    else if (closeSize && (!lineSize || close < line) && (!openSize || close < open))
    {
        type = lex_t::COMMENT_CLOSE;
        begin = close;
        end = begin + closeSize;
    }

    return type;
}

bool LexParser::CheckForOpenComments(size_t line)
{
    if (m_lexPosition.empty())
        return true;

    auto it = m_lexPosition.upper_bound(line);

    //LOG(DEBUG) << "  CheckForOpenRem line=" << line;
    if (!m_recursiveComment)
    {
        //C style
        while(--it != m_lexPosition.end())
        {
            auto[l, str] = *it;
            for (auto strIt = str.rbegin(); strIt != str.rend(); ++strIt)
            {
                if (*strIt == 'O')
                {
                    //LOG(DEBUG) << "  OpenComment for line=" << line << " at line=" << it->first;
                    return 0 != (m_commentOpen = 1);
                }
                else if (*strIt == 'C')
                    return 0 != (m_commentOpen = 0);
            }
        } 
    }
    else
    {
        //pascal style
        m_commentOpen = 0;
        while (--it != m_lexPosition.end())
        {
            auto[l, str] = *it;
            for (auto strIt = str.rbegin(); strIt != str.rend(); ++strIt)
            {
                if (*strIt == 'O')
                    ++m_commentOpen;
                else if (*strIt == 'C' && m_commentOpen > 0)
                    --m_commentOpen;
            }
        }
    }

    return 0 != (m_commentOpen = 0);
}

bool LexParser::ChangeStr(size_t line, const std::u16string& wstr, invalidate_t& inv)
{
    if (!m_scan)
        return true;

    LOG(DEBUG) << "LexParser::ChangeStr l=" << line;

    CheckForConcatenatedLine(line);
    CheckForOpenComments(line);

    std::string str = utf8::utf16to8(wstr);
    std::string lexstr;
    LexicalParse(str, lexstr);

    std::string prevLex;
    auto it = m_lexPosition.find(line);
    if (it != m_lexPosition.end())
        prevLex = it->second;

    if (lexstr != prevLex)
    {
        if (!lexstr.empty())
            m_lexPosition[line] = lexstr;
        else if (it != m_lexPosition.end())
            m_lexPosition.erase(it);

        bool comment{};
        bool backslashPrev{};
        bool backslashNew{};

        if (!prevLex.empty())
        {
            if (prevLex.find('O') != std::string::npos
                || prevLex.find('C') != std::string::npos)
            {
                comment = true;
            }
            if (prevLex.find('\\') != std::string::npos)
                backslashPrev = true;
        }
        if (!lexstr.empty())
        {
            if (lexstr.find('O') != std::string::npos
                || lexstr.find('C') != std::string::npos)
            {
                comment = true;
            }
            if (prevLex.find('\\') != std::string::npos)
                backslashNew = true;
        }

        if (comment || backslashPrev != backslashNew)
            //if comment changed or string Concatenated then invalidate full screen
            inv = invalidate_t::full;
    }
    else
        inv = invalidate_t::change;

    return true;
}

bool LexParser::AddStr(size_t line, const std::u16string& wstr, invalidate_t& inv)
{
    if (!m_scan)
        return true;

    LOG(DEBUG) << "LexParser::AddStr l=" << line;
    
    CheckForOpenComments(line);

    std::string str = utf8::utf16to8(wstr);
    std::string lexstr;
    LexicalParse(str, lexstr);

    if (!lexstr.empty())
    {
        if (lexstr.find('O') != std::string::npos
            || lexstr.find('C') != std::string::npos)
        {
            //if comment changed then invalidate full screen
            inv = invalidate_t::full;
        }
        AddLexem(line, lexstr);
    }
    else
        inv = invalidate_t::insert;

    return true;
}


bool LexParser::DelStr(size_t line, invalidate_t& inv)
{
    if (!m_scan)
        return true;

    LOG(DEBUG) << "LexParser::DelStr l=" << line;

    auto it = m_lexPosition.find(line);
    if (it != m_lexPosition.end())
    {
        const std::string& prevLex = it->second;
        if (prevLex.find('O') != std::string::npos
            || prevLex.find('C') != std::string::npos)
        {
            //if comment changed then invalidate full screen
            inv = invalidate_t::full;
        }
        DeleteLexem(line);
    }
    else
        inv = invalidate_t::del;

    return true;
}

bool LexParser::AddLexem(size_t line, const std::string& lexstr)
{
    for (auto it = m_lexPosition.rbegin(); it != m_lexPosition.rend();)
    {
        if (it->first < line)
            break;

        auto next = std::next(it);
        auto pos = m_lexPosition.extract(it->first);
        ++pos.key();
        auto[i, rc, node] = m_lexPosition.insert(std::move(pos));
        if (!rc)
        {
            _assert(0);
            return false;
        }
        
        it = next;
    }

    m_lexPosition[line] = lexstr;
    return true;
}

bool LexParser::DeleteLexem(size_t line)
{
    auto it = m_lexPosition.find(line);
    it = m_lexPosition.erase(it);
    for (; it != m_lexPosition.end(); ++it)
    {
        auto pos = m_lexPosition.extract(it);
        --pos.key();
        auto [i, rc, node] = m_lexPosition.insert(std::move(pos));
        if (!rc)
        {
            _assert(0);
            return false;
        }
        it = i;
    }

    return true;
}


#if 0

int LexBuff::DumpLPos()
{
  TPRINT(("\nDumpLPos %d\n", m_nPos));
//  for(int i = 0; i < m_nPos; ++i)
//    TPRINT(("LP%5d line=%04d t=%c\n", i, m_pLPos[i].nline, m_pLPos[i].type));
  return 0;
}


int LexBuff::CheckLexPair(const wchar* pStr, size_t* pLine, int* pX)
{
  TPRINT(("CheckLexPair\n"));

  if(!m_pLPos || !m_pParse)
    return 0;

  int   x = *pX;
  wchar c = pStr[x];

  if(c != '[' && c != ']'
  && c != '{' && c != '}'
  && c != '(' && c != ')'
  && c != '<' && c != '>')
    return 0;

  int i;
  char str[MAX_PARSE_STR];
  for(i = 0; pStr[i] && i < MAX_PARSE_STR; ++i)
    str[i] = wchar2char(m_nCP, pStr[i]);
  str[i] = 0;

  size_t nline = *pLine;
  int pos = (int)GetLexPos(nline);//pos used as signed in loop

  if(pos && m_pLPos[pos - 1].nline == nline - 1 && m_pLPos[pos - 1].type == '\\')
  {
    m_fCutLine = 1;
    m_fString  = m_pLPos[pos - 2].type;
  }
  else
  {
    m_fCutLine = 0;
    m_fString  = 0;
  }

  CheckForOpenRem(nline, pos);

  char lex[MAX_PARSE_STR];
  LexicalParse(nline, str, lex, NULL);

  TPRINT(("%s. x=%d\n", lex, x));
  if(lex[x] != '4')
    return 0;

  int fUp  = 0;
  int pair = 0;
  switch(c)
  {
  case '[':
    pair = ']';
    break;
  case '{':
    pair = '}';
    break;
  case '(':
    pair = ')';
    break;
  case ']':
    pair = '[';
    fUp = 1;
    break;
  case '}':
    pair = '{';
    fUp = 1;
    break;
  case ')':
    pair = '(';
    fUp = 1;
    break;
  case '<':
    pair = '>';
    break;
  case '>':
    pair = '<';
    fUp = 1;
    break;
  }

  int count = 1;
  if(!fUp)
  {
    ++x;
    //ищем в текущей строке
    while(pStr[x])
    {
      if(lex[x] == '4')
      {
        if(pStr[x] == pair)
          --count;
        else if(pStr[x] == c)
          ++count;
      }
      if(!count)
      {
        *pX = x;
        return 1;
      }
      ++x;
    }

    while(pos < m_nPos && m_pLPos[pos].nline == nline)
      ++pos;

    //ищем в следующих строках
    while(pos < m_nPos)
    {
      if(m_pLPos[pos].type == 'O')
      {
        //skip rem
        while(pos < m_nPos && m_pLPos[pos].type != 'C')
          ++pos;
      }
      else if(m_pLPos[pos].type == pair)
        --count;
      else if(m_pLPos[pos].type == c)
        ++count;

      if(!count)
        goto GetStartCount;

      ++pos;
    }
  }
  else
  {
    --x;
    //ищем в текущей строке
    while(x >= 0)
    {
      if(lex[x] == '4')
      {
        if(pStr[x] == pair)
          --count;
        else if(pStr[x] == c)
          ++count;
      }
      if(!count)
      {
        *pX = x;
        return 1;
      }
      --x;
    }

    while(pos >= 0 && m_pLPos[pos].nline == nline)
      --pos;

    //ищем в предыдущих строках
    while(pos >= 0)
    {
      if(m_pLPos[pos].type == 'C')
      {
        //skip rem
        TPRINT(("Scan skip rem pos=%d\n", pos));
        int open = pos;
        --pos;
        while(pos > 0 && m_pLPos[pos].type != 'C')
        {
          if(m_pLPos[pos].type == 'O')
            open = pos;
          --pos;
        }
        pos = open;
        TPRINT(("Skip rem pos=%d\n", pos));
      }
      else if(m_pLPos[pos].type == pair)
        --count;
      else if(m_pLPos[pos].type == c)
        ++count;

      if(!count)
        goto GetStartCount;

      --pos;
    }
  }

  return 0;

GetStartCount:
  *pLine = m_pLPos[pos].nline;

  //идем в начало строки и считаем скобки
  while(m_pLPos[pos].nline == *pLine)
  {
    if(m_pLPos[pos].type == pair)
      --count;
    else if(m_pLPos[pos].type == c)
      ++count;

    if(pos)
      --pos;
    else
      break;
  }
  *pX = count;//count на начало строки

  TPRINT(("l=%d x=%d\n", *pLine, *pX));

  return 1;
}


int LexBuff::GetLexPair(const wchar* pStr, size_t nline, wchar c, int* pX)
{
  TPRINT(("GetLexPair line=%d c=%c n=%d\n", nline, c, *pX));

  if(!m_pLPos || !m_pParse)
    return 0;

  int count = -*pX;

  int i;
  char str[MAX_PARSE_STR];
  for(i = 0; pStr[i] && i < MAX_PARSE_STR; ++i)
    str[i] = wchar2char(m_nCP, pStr[i]);
  str[i] = 0;

  size_t pos = GetLexPos(nline);
  if(pos && m_pLPos[pos - 1].nline == nline - 1 && m_pLPos[pos - 1].type == '\\')
  {
    m_fCutLine = 1;
    m_fString  = m_pLPos[pos - 2].type;
  }
  else
  {
    m_fCutLine = 0;
    m_fString  = 0;
  }

  CheckForOpenRem(nline, pos);

  char lex[MAX_PARSE_STR];
  LexicalParse(nline, str, lex, NULL);

  TPRINT(("%s.\n", lex));

  int fUp  = 0;
  int pair = 0;
  switch(c)
  {
  case '[':
    pair = ']';
    break;
  case '{':
    pair = '}';
    break;
  case '(':
    pair = ')';
    break;
  case ']':
    pair = '[';
    fUp  = 1;
    break;
  case '}':
    pair = '{';
    fUp  = 1;
    break;
  case ')':
    pair = '(';
    fUp  = 1;
    break;
  case '<':
    pair = '>';
    break;
  case '>':
    pair = '<';
    fUp  = 1;
    break;
  }

  int x = 0;
  while(pStr[x])
  {
    if(lex[x] == '4')
    {
      if(pStr[x] == pair)
      {
        int posx = x;
        if(!fUp)
          --count;
        else
        {
          ++x;
          while(pStr[x] && lex[x] != '4')
            ++x;

          if(pStr[x] != c)
          {
            --count;
            --x;
          }
        }

        if(!count)
        {
          x = posx;
          break;
        }
      }
      else if(pStr[x] == c)
        ++count;
    }
    ++x;
  }

  *pX = x;

  return 1;
}


int LexBuff::CheckFunc(size_t nline, const wchar* pStr)
{
  if(!m_pLPos || !m_pParse)
    return -1;

  int i;
  char str[MAX_PARSE_STR];
  for(i = 0; pStr[i] && i < MAX_PARSE_STR; ++i)
    str[i] = wchar2char(m_nCP, pStr[i]);
  str[i] = 0;
  TPRINT(("%s\n", str));

  char lex[MAX_PARSE_STR];
  LexicalParse(nline, str, lex, NULL);
  TPRINT(("%s\n", lex));

  if(!nline)
  {
    //начался новый поиск
    m_fcheck = 0;
    m_count  = 0;
    m_line   = -1;
  }

  int fFunc = -1;
  int fPrepr = 0;
  //мы ищем () {
  //или   { () {
  for(i = 0; str[i]; ++i)
  {
    if(lex[i] == '5')
    {
      if(!fPrepr && str[i] == '#')
      {
        //TPRINT(("Prepr %d\n", line));
        fPrepr = 1;
      }
    }
    else if(lex[i] == '4')
    {
      char c = str[i];
      if(c == '(')
      {
        if(m_fOldC && !fPrepr && !m_count)
          m_line = (int) nline;
        else if(m_line == -1 && !fPrepr && m_count == m_fcheck)
        {
          //TPRINT(("((( %d\n", line));
          m_line = (int) nline;
        }
      }
      else if(c == '{')
      {
        if(!m_count && m_line == -1 && !fPrepr && !m_fOldC)
          //пытаемся учесть внешние скобки
          m_fcheck = 1;

        ++m_count;
        if(m_count == m_fcheck + 1 && !fPrepr)
        {
          //TPRINT(("{{{ !!! %d\n", line));
          fFunc = m_line;
          m_line = -1;
        }
      }
      else if(c == '}')
      {
        //TPRINT(("}}} %d\n", line));
        if(m_count)
          --m_count;

        m_line = -1;

        if(!m_count)
          m_fcheck = 0;
      }
      else if(c == ';')
      {
        //TPRINT((";;; %d\n", line));
        if(!m_fOldC)
          m_line = -1;
      }
    }
  }

  return fFunc;
}

#endif