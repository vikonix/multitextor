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
#include "WndManager/ColorMap.h"
#include "Editor.h"
#include "utils/logger.h"
#include "utfcpp/utf8.h"
#include "utils/Directory.h"

namespace _Editor
{

std::map<std::string, LexConfig> LexParser::s_lexConfig
{
    {
        c_TextType,
        {
            //plain text
            c_TextType, //name
            "",         //file mask
            ",.:;=<>",  //delimiters
            "",         //name symbols
            {},         //special
            {},         //line
            {},         //open
            {},         //close
            {},         //toggled
            false,      //recursive
            false,      //not case
            false,      //save tab
            false,      //scan
            8,          //tab size
            {}          //key words
        }
    },
    {
        "C++",
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
            {},         //toggled
            false,      //recursive
            false,      //not case
            false,      //save tab
            true,       //scan
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
    },
    {
        "JSON",
        {
            //plain text
            "JSON",     //name
            "*.json",   //file mask
            ":,{}[]",   //delimiters
            "",         //name symbols
            {},         //special
            {"//"},     //line
            {"/*"},     //open
            {"*/"},     //close
            {},         //toggled
            false,      //recursive
            false,      //not case
            false,      //save tab
            true,       //scan
            4,          //tab size
            //key words
            {"true", "false"}
        }
    }
};

std::unordered_map<char16_t, std::pair<char16_t, bool>> LexParser::s_lexPairs
{
    {'[', {']', false}},
    {']', {'[', true}},
    {'{', {'}', false}},
    {'}', {'{', true}},
    {'(', {')', false}},
    {')', {'(', true}},
    {'<', {'>', false}},
    {'>', {'<', true}},
};

//////////////////////////////////////////////////////////////////////////////
bool LexParser::SetLexConfig(const LexConfig& config)
{
    s_lexConfig.insert_or_assign(config.langName, config);
    return true;
}

std::list<std::string> LexParser::GetFileTypeList()
{
    std::list<std::string> typeList;
    for (auto& [type, cfg] : s_lexConfig)
        typeList.emplace_back(type);
    return typeList;
}

std::pair<size_t, std::string> LexParser::GetFileType(const std::filesystem::path& name)
{
    size_t textType{};  //default type
    size_t lexType{};   //file type

    for (auto& [type, cfg] : s_lexConfig)
    {
        if (!name.empty() && !cfg.fileExt.empty())
        {
            std::stringstream ss(cfg.fileExt);
            std::string mask;

            while (std::getline(ss, mask, ';')) 
            {
                if (Directory::MatchMask(name, mask))
                {
                    return { lexType, type};
                }
            }
        }
        if (cfg.langName == c_TextType)
            textType = lexType;
        ++lexType;
    }

    return { textType, c_TextType };
}

//////////////////////////////////////////////////////////////////////////////
bool LexParser::SetParseStyle(const std::string& style)
{
    m_scan = false;
    m_parseStyle.clear();
    m_lexPosition.clear();

    m_special.clear();
    m_lineComment.clear();
    m_openComment.clear();
    m_closeComment.clear();
    m_keyWords.clear();

    auto FindStyle = [this](const std::string& style) {
        for (auto& [type, cfg] : s_lexConfig)
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

                for (auto& special : cfg.special)
                    m_special.insert(utf8::utf8to16(special));
                for (auto& lineComment : cfg.lineComment)
                    m_lineComment.insert(utf8::utf8to16(lineComment));
                for (auto& openComment : cfg.openComment)
                    m_openComment.insert(utf8::utf8to16(openComment));
                for (auto& closeComment : cfg.closeComment)
                    m_closeComment.insert(utf8::utf8to16(closeComment));
                for (auto& toggledComment : cfg.toggledComment)
                    m_toggledComment.insert(utf8::utf8to16(toggledComment));

                for (auto& kword : cfg.keyWords)
                    m_keyWords.insert(utf8::utf8to16(kword));

                m_recursiveComment = cfg.recursiveComment;
                m_notCase = cfg.notCase;
                m_saveTab = cfg.saveTab;
                m_tabSize = cfg.tabSize;
                m_scan    = cfg.scanFile;
                m_showTab = false;

                return true;
            }
        }
        return false;
    };

    if (!style.empty() && FindStyle(style))
        return true;

    return FindStyle(c_TextType);
}

//////////////////////////////////////////////////////////////////////////////
bool LexParser::ScanStr(size_t line, std::string_view str, [[maybe_unused]]const std::string& cp)
{
    if (!m_scan)
        return true;

    //LOG(DEBUG) << "ScanStr(" << line << ") '" << std::string(str) << "'";
    
    auto simpleConverter = [](std::string_view str) {
        std::u16string wstr;
        for (unsigned char c : str)
        {
            if (c < 0x80)
                wstr += c;
            else
                wstr += '_';
        }
        return wstr;
    };

    std::string lexstr;
    bool rc = LexicalParse(simpleConverter(str), lexstr);

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

    std::string lexstr;
    lexstr.reserve(strLen);
    LexicalParse(std::u16string_view(wstr).substr(0, strLen), lexstr, true);

    //LOG(DEBUG) << "GetColor(" << line << ") '" << str << "' len=" << len << " cut=" << m_cutLine << " strSymbol=" << m_stringSymbol.front();
    //LOG(DEBUG) << "  color='" << lex << "'";

    color.reserve(len);
    for (size_t i = 0; i < lexstr.size(); ++i)
        switch (lexstr[i])
        {
        case '0' + static_cast<char>(lex_t::BACKSLASH):
        case '0' + static_cast<char>(lex_t::OPERATOR):
        case 'K'://key word
            color.push_back(ColorWindowLKeyW);
            break;
        case 'R'://rem
            color.push_back(ColorWindowLRem);
            break;
        case 'N'://number
        case '0' + static_cast<char>(lex_t::STRING):
            color.push_back(ColorWindowLConst);
            break;
        case '0' + static_cast<char>(lex_t::DELIMITER):
            color.push_back(ColorWindowLDelim);
            break;
        default:
        //case ' '://space
        //case '0' + static_cast<char>(lex_t::SPACE)://space
        //case '0' + static_cast<char>(lex_t::SYMBOL)://symbol
            if (wstr[i] != S_TAB || !m_showTab)
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
    m_stringSymbol.clear();
    bool cutLine{};
    if (line)
    {
        auto prevIt = m_lexPosition.find(line - 1);
        if (prevIt != m_lexPosition.end())
        {
            //check for concatenated string with '\\' 
            auto&[prevLine, prevLex] = *prevIt;
            if (!prevLex.empty() && prevLex.back() == '\\')
            {
                cutLine = m_cutLine = true;
                m_stringSymbol.push_back(prevLex[prevLex.size() - 2]);
            }
        }
    }
    if (!cutLine)
    {
        m_cutLine = false;
    }
    
    return true;
}

bool LexParser::LexicalParse(std::u16string_view str, std::string& buff, bool color)
{
    if (!m_cutLine)
        m_stringSymbol.clear();

    m_cutLine = false;
    m_commentLine = false;

    if (str.empty())
    {
        buff.clear();
        return true;
    }

    char skipComment{};
    size_t skipCount{};

    size_t begin{};
    size_t end{};
    lex_t type;
    while ((type = LexicalScan(str, begin, end)) != lex_t::END)
    {
        //LOG(DEBUG) << "  lexem[" << end - begin + 1 << "]='" << std::string(str.substr(begin, end - begin + 1)) << "'";

        if (color && begin > 0)
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
                if (str[end] == '$' && str.size() > 2 && end < str.size() - 2)
                {
                    //LOG(DEBUG) << "    Check skip comment begin=" << begin << " end=" << end << " '" << std::string(str.substr(begin, end - begin + 1));
                    //bash var test
                    if (str[end + 1] == '(')
                        skipComment = ')';
                    else if (str[end + 1] == '{')
                        skipComment = '}';
                }
            }

            lex_t comment{};
            size_t e;
            if (ScanSpecial(str.substr(begin, end - begin + 1), e))
            {
                end = begin + e;
                offset = end;
            }
            else if ((comment = ScanCommentFromBegin(str.substr(begin, end - begin + 1), e)) != lex_t::END)
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
                    }

                    if (!m_commentLine && comment == lex_t::COMMENT_TOGGLED)
                    {
                        //LOG(DEBUG) << "    COMMENT_TOGGLED";
                        m_commentToggled = !m_commentToggled;
                        if (m_commentToggled)
                            ++m_commentOpen;
                        else
                            --m_commentOpen;
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
                        buff.resize(end + 1, 'R');
                    }
                    else
                    {
                        if (!m_commentLine && comment == lex_t::COMMENT_OPEN)
                        {
                            if (buff.empty() || buff.back() != 'O')
                                buff += 'O';
                        }
                        else if (!m_commentLine && comment == lex_t::COMMENT_TOGGLED)
                        {
                            if (buff.empty() || buff.back() != 'T')
                                buff += 'T';
                            else
                                buff.pop_back();
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
                    auto c = str[begin];
                    if (!color)
                    {
                        if (type == lex_t::DELIMITER)
                        {
                            if (c == '('
                             || c == '{'
                             || c == '['
                             || c == '<')
                                buff += static_cast<char>(c);
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
                                buff += static_cast<char>(m_stringSymbol.front());
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
                        if (IsNumeric(str.substr(begin, end - begin + 1)))
                            fill = 'N';
                        else if (IsKeyWord(str.substr(begin, end - begin + 1)))
                            fill = 'K';

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

                if (!m_commentLine && comment == lex_t::COMMENT_TOGGLED)
                {
                    //LOG(DEBUG) << "    COMMENT_TOGGLED1";
                    m_commentToggled = !m_commentToggled;
                    if (m_commentToggled)
                        ++m_commentOpen;
                    else
                        --m_commentOpen;
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
                    else if (!m_commentLine && comment == lex_t::COMMENT_TOGGLED)
                    {
                        if (buff.empty() || buff.back() != 'T')
                            buff += 'T';
                        else
                            buff.pop_back();
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

lex_t LexParser::SymbolType(char16_t c) const
{
    if (c < 0x80)
        return m_lexTab[c];
    else
        return lex_t::OTHER;
}

lex_t LexParser::LexicalScan(std::u16string_view str, size_t& begin, size_t& end)
{
    size_t strSize{ str.size() };
    lex_t type{};

    //skip leading space
    while (begin < strSize - 1 && (type = SymbolType(str[begin])) == lex_t::SPACE)
        ++begin;

    if (begin < strSize)
        type = SymbolType(str[begin]);

    end = begin;

    if (!m_stringSymbol.empty() && type != lex_t::END)
        //string continues from prev line
        type = lex_t::STRING;
    else if(!m_toggledComment.empty())
    {
        for (auto& comment : m_toggledComment)
        {
            if (auto pos = str.find(comment, begin); pos != std::string::npos)
            {
                if (pos == begin)
                {
                    //toggled comment
                    end += comment.size() - 1;
                    return lex_t::OPERATOR;
                }
            }
        }
    }

    switch (type)
    {
    case lex_t::BACKSLASH:
        m_cutLine = true;
        [[fallthrough]];
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
            if (m_stringSymbol.empty())
            {
                //begin of the string
                m_stringSymbol.push_back(str[begin]);
                if(end < strSize - 1)
                    ++end;
            }

            lex_t t;
            while (end < strSize - 1 && (t = SymbolType(str[end])) != lex_t::END)
            {
                if (t == lex_t::STRING)
                {
                    if constexpr (1)
                    {
                        //standard string
                        if (str[end] == m_stringSymbol.front())
                        {
                            m_stringSymbol.clear();
                            break;
                        }
                    }
                    else
                    {
                        //recursive string
                        if (str[end] == m_stringSymbol.back())
                        {
                            m_stringSymbol.pop_back();
                            if (m_stringSymbol.empty())
                            {
                                //end of string
                                break;
                            }
                        }
                        else
                            m_stringSymbol.push_back(str[end]);
                    }
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

bool LexParser::ScanSpecial(std::u16string_view lexem, size_t& end)
{
    for (auto& special : m_special)
    {
        if (lexem.find(special) == 0)
        {
            end = special.size() - 1;
            return true;
        }
    }
    
    return false;
}

bool LexParser::IsNumeric(std::u16string_view lexem)
{
    if ((lexem[0] >= '0' && lexem[0] <= '9') || lexem[0] == '#')
        return true;
    else
        return false;
}

bool LexParser::IsKeyWord(std::u16string_view lexem)
{
    if (m_keyWords.empty() || m_keyWords.find(std::u16string(lexem)) == m_keyWords.end())
        return false;
    else
        return true;
}

//line comment shields opened and hides closed
//first opened comment shields other opened comments
//closed comment always only one
lex_t LexParser::ScanCommentFromBegin(std::u16string_view lexem, size_t& end)
{
    if (!m_commentLine)
    {
        //check for toggled comment
        for (auto& comment : m_toggledComment)
        {
            if (auto pos = lexem.find(comment); pos != std::string::npos)
            {
                if (pos != 0)
                {
                    end = pos;
                    return lex_t::END;
                }
                end = comment.size() - 1;
                return lex_t::COMMENT_TOGGLED;
            }
        }

        //check for open comment
        for (auto& comment : m_openComment)
        {
            if (auto pos = lexem.find(comment); pos != std::string::npos)
            {
                if (pos != 0)
                {
                    end = pos;
                    return lex_t::END;
                }
                end = comment.size() - 1;
                return lex_t::COMMENT_OPEN;
            }
        }

        //check for line comment
        for (auto& comment : m_lineComment)
        {
            if (auto pos = lexem.find(comment); pos != std::string::npos)
            {
                if (pos != 0)
                {
                    end = pos;
                    return lex_t::END;
                }
                end = comment.size() - 1;
                return lex_t::COMMENT_LINE;
            }
        }
    }

    //check for close comment
    for (auto& comment : m_closeComment)
    {
        if (auto pos = lexem.find(comment); pos != std::string::npos)
        {
            if (pos != 0)
            {
                end = pos;
                return lex_t::END;
            }
            end = pos + comment.size() - 1;
            return lex_t::COMMENT_CLOSE;
        }
    }

    return lex_t::END;
}

lex_t LexParser::ScanComment(std::u16string_view lexem, size_t& begin, size_t& end)
{
    size_t line{};
    size_t open{};
    size_t close{};
    size_t toggled{};
    size_t lineSize{};
    size_t openSize{};
    size_t closeSize{};
    size_t toggledSize{};

    if (!m_commentLine)
    {
        //check for toggled comment
        for (auto& comment : m_toggledComment)
        {
            if ((toggled = lexem.find(comment)) != std::string::npos)
            {
                toggledSize = comment.size();
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

        //check for line comment
        for (auto& comment : m_lineComment)
        {
            if ((line = lexem.find(comment)) != std::string::npos)
            {
                lineSize = comment.size();
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
    else if (toggledSize && (!lineSize || toggled < line))
    {
        type = lex_t::COMMENT_TOGGLED;
        begin = toggled;
        end = begin + toggledSize;
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
    m_commentOpen = 0;
    m_commentToggled = false;

    if (m_lexPosition.empty())
        return false;

    //C style
    auto StdComment = [this](auto it) {
        auto& [l, str] = *it;
        for (auto strIt = str.rbegin(); strIt != str.rend(); ++strIt)
        {
            if (*strIt == 'O')
            {
                //LOG(DEBUG) << "  OpenComment for line=" << line << " at line=" << it->first;
                m_commentOpen = 1;
                return true;
            }
            else if (*strIt == 'C')
            {
                m_commentOpen = 0;
                return true;
            }
        }
        return false;
    };

    //pascal style
    auto RecursComment = [this](auto it) {
        auto& [l, str] = *it;
        for (auto strIt = str.rbegin(); strIt != str.rend(); ++strIt)
        {
            if (*strIt == 'O')
                ++m_commentOpen;
            else if (*strIt == 'C' && m_commentOpen > 0)
                --m_commentOpen;
        }
        return false;
    };

    //toggled style
    auto ToggledComment = [this](auto it) {
        auto& [l, str] = *it;
        for (auto strIt = str.begin(); strIt != str.end(); ++strIt)
        {
            if (*strIt == 'T')
            {
                m_commentToggled = !m_commentToggled;
                if (m_commentToggled)
                    ++m_commentOpen;
                else
                    --m_commentOpen;
            }
        }
        return false;
    };

    auto it = m_lexPosition.upper_bound(line);

    //LOG(DEBUG) << "  CheckForOpenRem line=" << line;
    while(it != m_lexPosition.begin())
    {
        --it;
        if (it->first >= line)
            continue;

        if (!m_recursiveComment)
        {
            if (StdComment(it))
                break;
            else if (ToggledComment(it))
                break;
        }
        else
        {
            if (RecursComment(it))
                break;
        }
    } 

    return m_commentOpen > 0;
}

bool LexParser::ChangeStr(size_t line, const std::u16string& wstr, invalidate_t& inv)
{
    inv = invalidate_t::change;
    if (!m_scan)
        return true;

    //LOG(DEBUG) << "LexParser::ChangeStr l=" << line;

    CheckForConcatenatedLine(line);
    CheckForOpenComments(line);

    std::string lexstr;
    LexicalParse(wstr, lexstr);

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
            if (   prevLex.find('O') != std::string::npos
                || prevLex.find('C') != std::string::npos
                || prevLex.find('T') != std::string::npos)
            {
                comment = true;
            }
            if (prevLex.find('\\') != std::string::npos)
                backslashPrev = true;
        }
        if (!lexstr.empty())
        {
            if (   lexstr.find('O') != std::string::npos
                || lexstr.find('C') != std::string::npos
                || lexstr.find('T') != std::string::npos)
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

    return true;
}

bool LexParser::AddStr(size_t line, const std::u16string& wstr, invalidate_t& inv)
{
    inv = invalidate_t::insert;
    if (!m_scan)
        return true;

    //LOG(DEBUG) << "LexParser::AddStr l=" << line;
    
    CheckForOpenComments(line);

    std::string lexstr;
    LexicalParse(wstr, lexstr);

    if (!lexstr.empty())
    {
        if (   lexstr.find('O') != std::string::npos
            || lexstr.find('C') != std::string::npos
            || lexstr.find('T') != std::string::npos)
        {
            //if comment changed then invalidate full screen
            inv = invalidate_t::full;
        }
    }

    AddLexem(line, lexstr);

    return true;
}


bool LexParser::DelStr(size_t line, invalidate_t& inv)
{
    inv = invalidate_t::del;
    if (!m_scan)
        return true;

    //LOG(DEBUG) << "LexParser::DelStr l=" << line;

    auto it = m_lexPosition.find(line);
    if (it != m_lexPosition.end())
    {
        const std::string& prevLex = it->second;
        if (   prevLex.find('O') != std::string::npos
            || prevLex.find('C') != std::string::npos
            || prevLex.find('T') != std::string::npos)
        {
            //if comment changed then invalidate full screen
            inv = invalidate_t::full;
        }
    }

    DeleteLexem(line);

    return true;
}

bool LexParser::AddLexem(size_t line, const std::string& lexstr)
{
    for (auto itr = m_lexPosition.rbegin(); itr != m_lexPosition.rend(); ++itr)
    {
        if (itr->first < line)
            break;

        auto pos = *itr;
        auto it = m_lexPosition.erase(--itr.base());
        itr = std::reverse_iterator(it);

        auto[i, rc] = m_lexPosition.emplace(pos.first + 1, pos.second);
        if (!rc)
        {
            _assert(0);
            return false;
        }
    }

    if(!lexstr.empty())
        m_lexPosition[line] = lexstr;

    return true;
}

bool LexParser::DeleteLexem(size_t line)
{
    auto it = m_lexPosition.find(line);
    if(it != m_lexPosition.end())
        m_lexPosition.erase(it);
    for (it = m_lexPosition.upper_bound(line); it != m_lexPosition.end(); ++it)
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

bool LexParser::CheckLexPair(const std::u16string& wstr, size_t& line, size_t& pos)
{
    //LOG(DEBUG) << "CheckLexPair";

    if(!m_scan)
        return false;

    char16_t ch = wstr[pos];

    auto pair = s_lexPairs.find(ch);
    if(pair == s_lexPairs.end())
        return false;

    CheckForConcatenatedLine(line);
    CheckForOpenComments(line);

    std::string curLex;
    LexicalParse(wstr, curLex, true);
    const char delimiter{ '0' + static_cast<char>(lex_t::DELIMITER) };

    //LOG(DEBUG) << "    lex=" << curLex << ". pos=" << pos;
    if(curLex[pos] != delimiter)
        return false;

    auto& [chPair, up] = pair->second;
    auto posIt = m_lexPosition.lower_bound(line);

    int count = 1;
    if(!up)
    {
        ++pos;
        //looking in current line
        for(; pos < wstr.size(); ++pos)
        {
            if(curLex[pos] == delimiter)
            {
                if(wstr[pos] == chPair)
                    --count;
                else if(wstr[pos] == ch)
                    ++count;
            }
            if(!count)
            {
                return true;
            }
        }

        //looking in next lines
        bool skipComment{};
        for (; posIt != m_lexPosition.end(); ++posIt)
        {
            if (posIt->first == line)
                continue;
            for (auto c : posIt->second)
            {
                if (c == 'O')
                {
                    //skip comment
                    skipComment = true;
                }
                else if (skipComment)
                {
                    if (c == 'C')
                        skipComment = false;
                }
                else if (c == chPair)
                    --count;
                else if (c == ch)
                    ++count;

                if (!count)
                    goto GetStartCount;
            }
        }
    }
    else
    {
        --pos;
        //looking in current line
        for (; pos < wstr.size(); --pos)
        {
            if (curLex[pos] == delimiter)
            {
                if (wstr[pos] == chPair)
                    --count;
                else if (wstr[pos] == ch)
                    ++count;
            }
            if (!count)
            {
                return true;
            }
        }

        //looking in prev lines
        for (; posIt != m_lexPosition.end(); --posIt)
        {
            if (posIt->first == line)
                continue;

            auto lex = posIt->second;
            for (size_t i = lex.size() - 1; i < lex.size(); --i)//go through 0
            {
                char c = lex[i];
                if (c == 'C')
                {
                    auto saveIt{ m_lexPosition.end() };
                    size_t saveI{};
                    bool end{};

                    //LOG(DEBUG) << "Scan skip rem line=" << posIt->first << " pos=" << i;
                    for (; posIt != m_lexPosition.end() && !end; --posIt)
                    {
                        const auto& lex1 = posIt->second;
                        for (--i; i < lex1.size(); --i)
                        {
                            char c1 = lex1[i];
                            if (c1 == 'O')
                            {
                                saveIt = posIt;
                                saveI = i;
                            }
                            else if (c1 == 'C')
                            {
                                end = true;
                                break;
                            }
                        }
                    }

                    if (saveIt == m_lexPosition.end())
                        return false;

                    posIt = saveIt;
                    i = saveI;
                    lex = saveIt->second;
                    //LOG(DEBUG) << "Skip rem line=" << posIt->first << " pos=" << i;
                }
                else if (c == chPair)
                    --count;
                else if (c == ch)
                    ++count;

                if (!count)
                    goto GetStartCount;
            }
        }
    }
    return false;

GetStartCount:
    line = posIt->first;
    const auto& lex = posIt->second;

    //goto begin of line and count brackets
    for (size_t i = lex.size() - 1; i < lex.size(); --i)//go through 0
    {
        char c = lex[i];
        if (c == chPair)
            --count;
        else if (c == ch)
            ++count;
    }

    pos = static_cast<size_t>(count);

    //LOG(DEBUG) << "line=" << line << " pos=" << count;
    return true;
}

bool LexParser::GetLexPair(const std::u16string& wstr, size_t line, char16_t ch, size_t& pos)
{
    int count = -static_cast<int>(pos);

    //LOG(DEBUG) << "GetLexPair line=" << line << " ch=" << ch << " count=" << count;

    if (!m_scan || m_lexPosition.empty())
        return false;

    auto pair = s_lexPairs.find(ch);
    if (pair == s_lexPairs.end())
        return false;

    CheckForConcatenatedLine(line);
    CheckForOpenComments(line);

    std::string lex;
    LexicalParse(wstr, lex, true);
    const char delimiter{ '0' + static_cast<char>(lex_t::DELIMITER) };
    
    //LOG(DEBUG) << "    lex=" << lex;
    auto& [chPair, up] = pair->second;

    size_t x{};
    for(; x < wstr.size(); ++x)
    {
        if(lex[x] == delimiter)
        {
            if(wstr[x] == chPair)
            {
                size_t posx = x;
                if(!up)
                    --count;
                else
                {
                    ++x;
                    while(x < wstr.size() && lex[x] != delimiter)
                        ++x;

                    if(wstr[x] != ch)
                    {
                        --count;
                        --x;
                    }
                }

                if(!count)
                {
                    pos = posx;
                    break;
                }
            }
            else if(wstr[x] == ch)
                ++count;
        }
    }

    return true;
}

} //namespace _Editor
