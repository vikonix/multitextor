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

#include "utils/logger.h"
#include "nlohmann/json.hpp"
#include "Config.h"
#include "EditorApp.h"

#include <fstream>

namespace _Editor
{

EditorConfig g_editorConfig;

bool EditorConfig::Load(const path_t& file)
{
    std::ifstream ifs(file);
    nlohmann::json json = nlohmann::json::parse(ifs);
    auto& jsonConfig = json[ConfigKey];

    EditorConfig config;
    config.colorFile        = jsonConfig[ColorKey];
    config.keyFile          = jsonConfig[KeymapKey];
    config.showAccessMenu   = jsonConfig[ShowAccessMenuKey];
    config.showClock        = jsonConfig[ShowClockKey];
    config.fileSaveTime     = jsonConfig[FileSaveTimeKey];
    config.maxScreenSize    = jsonConfig[MaxScreenSizeKey];

    colorFile       = config.colorFile;
    keyFile         = config.keyFile;
    showAccessMenu  = config.showAccessMenu;
    showClock       = config.showClock;
    fileSaveTime    = config.fileSaveTime;
    maxScreenSize   = config.maxScreenSize;

    return true;
}

bool EditorConfig::Save(const path_t& file, bool force)
{
    if (!m_changed && !force)
        return true;

    nlohmann::json json;
    json[ColorKey]          = colorFile;
    json[KeymapKey]         = keyFile;
    json[ShowAccessMenuKey] = showAccessMenu;
    json[ShowClockKey]      = showClock;
    json[FileSaveTimeKey]   = fileSaveTime;
    json[MaxScreenSizeKey]  = maxScreenSize;

    nlohmann::json jsonConfig;
    jsonConfig[ConfigKey] = json;

    std::ofstream ofs(file);
    ofs << jsonConfig.dump(2);
    LOG(DEBUG) << jsonConfig.dump(2);

    return true;
}

bool KeyConfig::Load(const path_t& file)
{
    std::ifstream ifs(file);
    nlohmann::json json = nlohmann::json::parse(ifs);
    auto& jsonConfig = json[ConfigKey];

    auto& app = Application::getInstance();
    CmdMap map;
    for (auto&& entry : jsonConfig)
    {
        std::vector<input_t> keyArray;
        auto& keys = entry[KeyKey];
        for (auto& key : keys)
            keyArray.push_back(app.GetCode(key));

        std::vector<input_t> cmdArray;
        auto& cmds = entry[CmdKey];
        for (auto&& cmd : cmds)
            cmdArray.push_back(app.GetCode(cmd));
        
        map.push_back({keyArray, cmdArray});
    }

    return true;
}

bool KeyConfig::Save(const path_t& file)
{
    auto& app = Application::getInstance();

    nlohmann::json json;
    for (const auto& map : { g_defaultAppKeyMap, g_defaultEditKeyMap })
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
            json.push_back(entry);
        }

    nlohmann::json jsonConfig;
    jsonConfig[ConfigKey] = json;

    std::ofstream ofs(file);
    ofs << jsonConfig.dump(2);
    LOG(DEBUG) << jsonConfig.dump(2);

    return true;
}


} //namespace _Editor
