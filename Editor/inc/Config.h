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
#include "utils/Directory.h"
#include "nlohmann/json.hpp"

#include <string>
#include <set>

using namespace _Utils;
using namespace _Console;

namespace _Editor
{

class EditorConfig
{
    inline static const std::string ConfigKey           { "EditorConfig" };
    inline static const std::string ColorKey            { "1_ColorFile" };
    inline static const std::string KeymapKey           { "2_KeyMapFile" };
    inline static const std::string ShowAccessMenuKey   { "ShowAccessMenu" };
    inline static const std::string ShowClockKey        { "ShowClock" };
    inline static const std::string FileSaveTimeKey     { "FileSaveTime" };
    inline static const std::string MaxScreenSizeKey    { "MaxScreenSize" };

public:
    inline static const std::string ConfigDir           { "config" };
    inline static const std::string ConfigFile          { "multitextor.json" };

    std::string colorFile       {"default.clr"};
    std::string keyFile         {"default.kmap"};
    uint32_t    fileSaveTime    {0};
    bool        showAccessMenu  {true};
    bool        showClock       {true};
    bool        maxScreenSize   {false};

    bool        m_changed{};

    bool Load(const path_t& file);
    bool Save(const path_t& file, bool force = false);
};


class KeyConfig
{
    inline static const std::string ConfigKey   { "KeyConfig" };
    inline static const std::string AppKey      { "AppKeys" };
    inline static const std::string EditorKey   { "EditorKeys" };
    inline static const std::string KeyKey      { "Key" };
    inline static const std::string CmdKey      { "Cmd" };

public:
    bool Load(const path_t& file);
    bool Save(const path_t& file);
};

struct LexConfig;
class ParserConfig
{
    inline static const std::string ConfigKey           { "ParserConfig" };
    inline static const std::string LangNameKey         { "1_LanguageName" };
    inline static const std::string FileExtKey          { "2_FileExtentions" };
    inline static const std::string DelimitersKey       { "3_Delimiters" };
    inline static const std::string NameSymbolsKey      { "4_NameSymbols" };
    inline static const std::string SpecialSymbolsKey   { "5_SpecialSymbols" };
    inline static const std::string LineCommentsKey     { "6_LineComments" };
    inline static const std::string OpenCommentsKey     { "7_OpenComments" };
    inline static const std::string CloseCommentsKey    { "8_ClosedComments" };
    inline static const std::string RecursiveCommentsKey{ "RecursiveComments" };
    inline static const std::string ToggledCommentsKey  { "ToggledComments" };
    inline static const std::string NotCaseKey          { "NotCase" };
    inline static const std::string SaveTabsKey         { "SaveTabs" };
    inline static const std::string TabSizeKey          { "TabSize" };
    inline static const std::string KeywordsKey         { "_KeyWords" };

public:
    inline static const std::string ConfigDir{ "parser" };
    inline static const std::string Ext{ ".lex" };

    bool Load(const path_t& file);
    bool Save(const path_t& file, const LexConfig& config);
};

class WndConfig
{
    inline static const std::string FilePathKey { "FilePath" };
    inline static const std::string FirstLineKey{ "FirstLine" };
    inline static const std::string XOffsetKey  { "XOffset" };
    inline static const std::string CursorXKey  { "CursorX" };
    inline static const std::string CursorYKey  { "CursorY" };
    inline static const std::string ROKey       { "ro" };
    inline static const std::string LogKey      { "log" };
    inline static const std::string TabSizeKey  { "TabSize" };
    inline static const std::string SaveTabsKey { "SaveTabs" };
    inline static const std::string EolKey      { "Eol" };
    inline static const std::string CodePageKey { "CodePage" };
    inline static const std::string ParserKey   { "Parser" };

public:
    std::string filePath;
    std::string parser{};
    std::string cp{};
    bool        ro{};
    bool        log{};

    size_t      firstLine{};
    size_t      xOffset{};
    pos_t       cursorX{};
    pos_t       cursorY{};
    size_t      tabSize{4};
    bool        saveTabs{};
    size_t      eol{};

    bool Load(const nlohmann::json& json);
    bool Save(nlohmann::json& json) const;
};

class ViewConfig
{
    inline static const std::string SizeXKey        { "SizeX" };
    inline static const std::string SizeYKey        { "SizeY" };
    inline static const std::string TypeKey         { "Type" };
    inline static const std::string ActiveKey       { "Active" };
    inline static const std::string File1Key        { "File1" };
    inline static const std::string File2Key        { "File2" };

public:

    pos_t       sizex{};
    pos_t       sizey{};
    size_t      type{};
    int         active{};
    std::string file1;
    std::string file2;

    bool Load(const nlohmann::json& json);
    bool Save(nlohmann::json& json) const;
};

class DialogsConfig
{
    inline static const std::string ConfigKey   { "DialogsConfig" };
    inline static const std::string FilePathKey { "FilePath" };
    inline static const std::string MaskKey     { "MaskList" };
    inline static const std::string FindKey     { "FindList" };
    inline static const std::string ReplaceKey  { "ReplaceList" };

public:
    std::string             filePath;
    std::list<std::string>  fileMaskList;
    std::set<std::string>   findList;
    std::set<std::string>   replaceList;

    bool Load(const nlohmann::json& json);
    bool Save(nlohmann::json& json) const;
};

class SessionConfig
{
    inline static const std::string ConfigKey       { "MultitextorSessionConfig" };
    inline static const std::string WndKey          { "WndList" };
    inline static const std::string ViewKey         { "View" };
    
    inline static const std::string RecentFilesKey  { "RecentFiles" };
    inline static const std::string DialogsKey      { "Dialogs" };

    nlohmann::json m_json;

public:
    inline static const std::string File{ ".m.smt" };

    bool SaveConfig(const WndConfig& config);
    bool SaveConfig(const ViewConfig& config);
    bool SaveConfig(const DialogsConfig& config);

    bool Load(const path_t& file);
    bool Save(const path_t& file);
};

extern class EditorConfig g_editorConfig;

} //namespace _Editor
