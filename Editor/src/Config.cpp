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
#include "utils/logger.h"
#include "Config.h"
#include "EditorApp.h"
#include "Dialogs/EditorDialogs.h"

#include <fstream>

namespace _Editor
{

EditorConfig g_editorConfig;

bool EditorConfig::Load(const path_t& file)
{
    std::ifstream ifs(file);
    if (!ifs)
        return true;

    LOG(DEBUG) << "Load " << file.u8string();
    nlohmann::json json = nlohmann::json::parse(ifs);
    auto& jsonConfig = json[ConfigKey];

    //first collect all variables
    EditorConfig config;
    config.colorFile        = jsonConfig[ColorKey];
    config.keyFile          = jsonConfig[KeymapKey];
    config.showAccessMenu   = jsonConfig[ShowAccessMenuKey];
    config.showClock        = jsonConfig[ShowClockKey];
    config.fileSaveTime     = jsonConfig[FileSaveTimeKey];

    colorFile       = config.colorFile;
    keyFile         = config.keyFile;
    showAccessMenu  = config.showAccessMenu;
    showClock       = config.showClock;
    fileSaveTime    = config.fileSaveTime;

    return true;
}

bool EditorConfig::Save(const path_t& file, bool force)
{
    if (!m_changed && !force)
        return true;

    LOG(DEBUG) << "Save " << file.u8string();
    nlohmann::json json;
    json[ColorKey]          = colorFile;
    json[KeymapKey]         = keyFile;
    json[ShowAccessMenuKey] = showAccessMenu;
    json[ShowClockKey]      = showClock;
    json[FileSaveTimeKey]   = fileSaveTime;

    nlohmann::json jsonConfig;
    jsonConfig[ConfigKey] = json;

    std::ofstream ofs(file);
    ofs << jsonConfig.dump(2);
    //LOG(DEBUG) << jsonConfig.dump(2);

    m_changed = false;

    return true;
}

bool KeyConfig::Load(const path_t& file)
{
    std::ifstream ifs(file);
    if (!ifs)
        return true;

    LOG(DEBUG) << "Load " << file.u8string();
    nlohmann::json json = nlohmann::json::parse(ifs);
    auto& jsonConfig = json[ConfigKey];

    auto& app = Application::getInstance();
    bool editorMap{};
    for (auto& jsonMap : { jsonConfig[AppKey], jsonConfig[EditorKey] })
    {
        CmdMap map;
        for (auto&& entry : jsonMap)
        {
            std::vector<input_t> keyArray;
            auto& keys = entry[KeyKey];
            for (auto& key : keys)
                keyArray.push_back(app.GetCode(key));

            std::vector<input_t> cmdArray;
            auto& cmds = entry[CmdKey];
            for (auto&& cmd : cmds)
                cmdArray.push_back(app.GetCode(cmd));

            map.push_back({ keyArray, cmdArray });
        }
        if (!editorMap)
        {
            editorMap = true;
            g_AppKeyMap = map;
        }
        else
        {
            g_EditKeyMap = map;
        }
    }

    return true;
}

bool KeyConfig::Save(const path_t& file) const
{
    auto& app = Application::getInstance();

    nlohmann::json json;
    bool editorMap{};
    for (const auto& map : { g_AppKeyMap, g_EditKeyMap })
    {
        nlohmann::json jsonMap;
        for (const auto& [keys, cmds] : map)
        {
            nlohmann::json key;
            nlohmann::json cmd;

            for (auto k : keys)
                key.push_back(app.GetCodeName(k));
            for (auto c : cmds)
                cmd.push_back(app.GetCodeName(c));

            nlohmann::json entry;
            entry[KeyKey] = key;
            entry[CmdKey] = cmd;
            jsonMap.push_back(entry);
        }
        if (!editorMap)
        {
            editorMap = true;
            json[AppKey] = jsonMap;
        }
        else
        {
            json[EditorKey] = jsonMap;
        }
    }

    nlohmann::json jsonConfig;
    jsonConfig[ConfigKey] = json;

    std::ofstream ofs(file);
    ofs << jsonConfig.dump(2);
    //LOG(DEBUG) << jsonConfig.dump(2);

    return true;
}

bool ParserConfig::Load(const path_t& file)
{
    std::ifstream ifs(file);
    if (!ifs)
        return true;

    LOG(DEBUG) << "Load " << file.u8string();
    nlohmann::json jsonConfig = nlohmann::json::parse(ifs);
    auto& json = jsonConfig[ConfigKey];

    LexConfig config;
    config.langName         = json[LangNameKey];
    config.fileExt          = json[FileExtKey];
    config.delimiters       = json[DelimitersKey];
    config.nameSymbols      = json[NameSymbolsKey];
    config.recursiveComment = json[RecursiveCommentsKey];
    config.toggledComment   = json[ToggledCommentsKey];
    config.notCase          = json[NotCaseKey];
    config.saveTab          = json[SaveTabsKey];
    config.tabSize          = json[TabSizeKey];

    for(auto& entry : json[SpecialSymbolsKey])
        config.special.push_back(static_cast<std::string>(entry));
    for (auto& entry : json[LineCommentsKey])
        config.lineComment.push_back(static_cast<std::string>(entry));
    for (auto& entry : json[OpenCommentsKey])
        config.openComment.push_back(static_cast<std::string>(entry));
    for (auto& entry : json[CloseCommentsKey])
        config.closeComment.push_back(static_cast<std::string>(entry));
    for (auto& entry : json[KeywordsKey])
        config.keyWords.insert(static_cast<std::string>(entry));

    LexParser::SetLexConfig(config);

    return true;
}

bool ParserConfig::Save(const path_t& file, const LexConfig& config) const
{
    nlohmann::json json;

    json[LangNameKey]           = config.langName;
    json[FileExtKey]            = config.fileExt;
    json[DelimitersKey]         = config.delimiters;
    json[NameSymbolsKey]        = config.nameSymbols;
    json[SpecialSymbolsKey]     = config.special;
    json[LineCommentsKey]       = config.lineComment;
    json[OpenCommentsKey]       = config.openComment;
    json[CloseCommentsKey]      = config.closeComment;
    json[RecursiveCommentsKey]  = config.recursiveComment;
    json[ToggledCommentsKey]    = config.toggledComment;
    json[NotCaseKey]            = config.notCase;
    json[SaveTabsKey]           = config.saveTab;
    json[TabSizeKey]            = config.tabSize;
    json[KeywordsKey]           = config.keyWords;

    nlohmann::json jsonConfig;
    jsonConfig[ConfigKey] = json;

    std::ofstream ofs(file);
    ofs << jsonConfig.dump(2);
    //LOG(DEBUG) << jsonConfig.dump(2);

    return true;
}

bool FileConfig::Load(const nlohmann::json& json)
{
    filePath    = json[FilePathKey];
    parser      = json[ParserKey];
    cp          = json[CodePageKey];
    ro          = json[ROKey];
    log         = json[LogKey];

    return true;
}

bool FileConfig::Save(nlohmann::json& json) const
{
    json[FilePathKey]   = filePath;
    json[ParserKey]     = parser;
    json[CodePageKey]   = cp;
    json[ROKey]         = ro;
    json[LogKey]        = log;

    return true;
}

bool WndConfig::Load(const nlohmann::json& json)
{
    filePath    = json[FilePathKey];
    parser      = json[ParserKey];
    cp          = json[CodePageKey];
    ro          = json[ROKey];
    log         = json[LogKey];

    firstLine   = json[FirstLineKey];
    xOffset     = json[XOffsetKey];
    cursorX     = json[CursorXKey];
    cursorY     = json[CursorYKey];
    tabSize     = json[TabSizeKey];
    saveTabs    = json[SaveTabsKey];
    eol         = json[EolKey];

    return true;
}

bool WndConfig::Save(nlohmann::json& json) const
{
    json[FilePathKey]   = filePath;
    json[ParserKey]     = parser;
    json[CodePageKey]   = cp;
    json[ROKey]         = ro;
    json[LogKey]        = log;

    json[FirstLineKey]  = firstLine;
    json[XOffsetKey]    = xOffset;
    json[CursorXKey]    = cursorX;
    json[CursorYKey]    = cursorY;
    json[TabSizeKey]    = tabSize;
    json[SaveTabsKey]   = saveTabs;
    json[EolKey]        = eol;

    return true;
}

bool ViewConfig::Load(const nlohmann::json& json)
{
    sizex   = json[SizeXKey];
    sizey   = json[SizeYKey];
    type    = json[TypeKey];
    active  = json[ActiveKey];
    file1   = json[File1Key];
    file2   = json[File2Key];

    return true;
}

bool ViewConfig::Save(nlohmann::json& json) const
{
    json[SizeXKey]  = sizex;
    json[SizeYKey]  = sizey;
    json[TypeKey]   = type;
    json[ActiveKey] = active;
    json[File1Key]  = file1;
    json[File2Key]  = file2;

    return true;
}

bool DialogsConfig::Load(const nlohmann::json& json)
{
    filePath = json[FilePathKey];
    for(auto& var : json[MaskKey])
        fileMaskList.emplace_back(var);
    for(auto& var : json[FindKey])
        findList.emplace(var);
    for(auto& var : json[ReplaceKey])
        replaceList.emplace(var);

    return true;
}

bool DialogsConfig::Save(nlohmann::json& json) const
{
    json[FilePathKey]   = filePath;
    json[MaskKey]       = fileMaskList;
    json[FindKey]       = findList;
    json[ReplaceKey]    = replaceList;

    return true;
}

bool SessionConfig::SaveConfig(const WndConfig& config)
{
    nlohmann::json json;
    config.Save(json);
    m_json[ConfigKey][WndKey].push_back(json);
    return true;
}

bool SessionConfig::SaveConfig(const FileConfig& config)
{
    nlohmann::json json;
    config.Save(json);
    m_json[ConfigKey][RecentFilesKey].push_back(json);
    return true;
}

bool SessionConfig::SaveConfig(const ViewConfig& config)
{
    nlohmann::json json;
    config.Save(json);
    m_json[ConfigKey][ViewKey] = json;
    return true;
}

bool SessionConfig::SaveConfig(const DialogsConfig& config)
{
    nlohmann::json json;
    config.Save(json);
    m_json[ConfigKey][DialogsKey] = json;
    return true;
}

template <>
std::vector<WndConfig> SessionConfig::GetConfig< std::vector<WndConfig>>() const
{
    std::vector<WndConfig> config;
    for (auto& entry : m_json.at(ConfigKey).at(WndKey))
    {
        WndConfig wnd;
        wnd.Load(entry);
        config.push_back(std::move(wnd));
    }
    return config;
}

template <>
std::vector<FileConfig> SessionConfig::GetConfig< std::vector<FileConfig>>() const
{
    std::vector<FileConfig> config;
    for (auto& entry : m_json.at(ConfigKey).at(RecentFilesKey))
    {
        FileConfig wnd;
        wnd.Load(entry);
        config.push_back(std::move(wnd));
    }
    return config;
}

template <>
ViewConfig SessionConfig::GetConfig<ViewConfig>() const
{
    ViewConfig config;
    config.Load(m_json.at(ConfigKey).at(ViewKey));
    return config;
}

template <>
DialogsConfig SessionConfig::GetConfig<DialogsConfig>() const
{
    DialogsConfig config;
    config.Load(m_json.at(ConfigKey).at(DialogsKey));
    return config;
}

bool SessionConfig::Save(const path_t& file) const
{
    LOG(DEBUG) << "Save " << file.u8string();
    std::ofstream ofs(file);
    if (!ofs)
        return true;

    ofs << m_json.dump(2);
    //LOG(DEBUG) << m_json.dump(2);

    return true;
}

bool SessionConfig::Load(const path_t& file)
{
    std::ifstream ifs(file);
    if (!ifs)
        return true;

    LOG(DEBUG) << "Load " << file.u8string();
    m_json = nlohmann::json::parse(ifs);

    return true;
}

} //namespace _Editor
