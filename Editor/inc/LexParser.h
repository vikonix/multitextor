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
#pragma once

#include "Console/Types.h"
#include "WndManager/Invalidate.h"
#include "utils/SymbolType.h"

#include <string>
#include <map>
#include <optional>
#include <unordered_set>
#include <unordered_map>

using namespace _Utils;
using namespace _Console;
using namespace _WndManager;

namespace _Editor
{

//symbol_t types
enum class lex_t
{
    END             = static_cast<int>(symbol_t::eol),      //0  
    SPACE           = static_cast<int>(symbol_t::blank),    //1  
    STRING          = static_cast<int>(symbol_t::quote),    //2    
    BACKSLASH       = static_cast<int>(symbol_t::backslash),//3
    ERROR           = static_cast<int>(symbol_t::cntrl),    //4
    OPERATOR        = static_cast<int>(symbol_t::punct),    //5
    SYMBOL          = static_cast<int>(symbol_t::alnum),    //6
    OTHER           = static_cast<int>(symbol_t::other),    //7
    DELIMITER       = static_cast<int>(symbol_t::max),      //8...
    
    COMMENT_OPEN,
    COMMENT_CLOSE,
    COMMENT_LINE
};

struct LexConfig 
{
    std::string langName;
    std::string fileExt;

    std::string delimiters;
    std::string nameSymbols;
    
    std::list<std::string> special;            //some special symbols combination like $# /# perl sh bash
    std::list<std::string> lineComment;
    std::list<std::string> openComment;
    std::list<std::string> closeComment;
  
    bool        recursiveComment{}; //like pascal
    bool        toggledComment{};   //like asn1 -- comment --
    bool        notCase{};          //key words not case sensitive
    bool        saveTab{};
    size_t      tabSize{8};

    std::unordered_set<std::string> keyWords;
};


//////////////////////////////////////////////////////////////////////////////
using string_set = std::unordered_set<std::u16string>;
class LexParser
{
    static std::unordered_map<std::string, LexConfig> s_lexConfig;
    static std::unordered_map<char16_t, std::pair<char16_t, bool>> s_lexPairs;

    bool        m_scan{};
    std::string m_parseStyle;

    static const inline size_t lexTabSize = 0x80;
    lex_t       m_lexTab[lexTabSize]{};

    string_set  m_special;
    string_set  m_lineComment;
    string_set  m_openComment;
    string_set  m_closeComment;
    string_set  m_keyWords;

    bool        m_recursiveComment{};
    bool        m_toggledComment{};
    bool        m_notCase{};
    bool        m_saveTab{};
    size_t      m_tabSize{8};

    bool        m_showTab{};

    std::map<size_t, std::string> m_lexPosition;
    char16_t    m_stringSymbol{};
    bool        m_cutLine{};
    bool        m_commentLine{};
    size_t      m_commentOpen{};

    //function check
//    bool        m_funcCheck{};
//    size_t      m_funcLine{};
//    size_t      m_curlyBracketCount{}; //{

protected:
    bool    CheckForOpenComments(size_t line);
    bool    CheckForConcatenatedLine(size_t line);
    
    bool    AddLexem(size_t line, const std::string& lexstr);
    bool    DeleteLexem(size_t line);

    lex_t   SymbolType(char16_t c) const ;
    lex_t   ScanCommentFromBegin(std::u16string_view lexem, size_t& end);
    lex_t   ScanComment(std::u16string_view lexem, size_t& begin, size_t& end);
    bool    ScanSpecial(std::u16string_view lexem, size_t& end);
    bool    IsNumeric(std::u16string_view lexem);
    bool    IsKeyWord(std::u16string_view lexem);

    lex_t   LexicalScan(std::u16string_view str, size_t& begin, size_t& end);
    bool    LexicalParse(std::u16string_view str, std::string& buff, bool color = false);//if color is true scan for color scheme

public:
    LexParser() = default;
    ~LexParser() = default;

    static bool SetLexConfig(const std::list<LexConfig>& config);
    static std::list<std::string> GetFileTypeList();
    static size_t CheckFileName(const std::string& name);

    bool    EnableParsing(bool scan)    { return m_scan = scan; }
    bool    SetParseStyle(const std::string& style = "");
    std::string GetParseStyle() const   {return m_parseStyle;}

    bool    GetShowTab() const          {return m_showTab;};
    bool    SetShowTab(bool show)       {return m_showTab = show;};
    bool    GetSaveTab() const          {return m_saveTab;}
    size_t  GetTabSize() const          {return m_tabSize;}

    bool    Clear() { m_lexPosition.clear(); return true; }
    bool    ScanStr(size_t line, std::string_view str, const std::string& cp);
    bool    GetColor(size_t line, const std::u16string& str, std::vector<color_t>& color, size_t len);

    bool    CheckLexPair(const std::u16string& str, size_t& line, size_t& pos);
    bool    GetLexPair(const std::u16string& str, size_t line, char16_t c, size_t& pos);

    bool    ChangeStr(size_t line, const std::u16string& str, invalidate_t& inv);
    bool    AddStr(size_t line, const std::u16string& str, invalidate_t& inv);
    bool    DelStr(size_t line, invalidate_t& inv);

    //std::optional<size_t> CheckFunc(size_t line, const std::u16string& str);
};

} //namespace _Editor
