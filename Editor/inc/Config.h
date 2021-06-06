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

#include <string>

using namespace _Utils;
using namespace _Console;

namespace _Editor
{

class EditorConfig
{
    inline static const std::string ConfigKey           { "EditorConfig" };
    inline static const std::string ColorKey            { "ColorFile" };
    inline static const std::string KeymapKey           { "KeyMapFile" };
    inline static const std::string ShowAccessMenuKey   { "ShowAccessMenu" };
    inline static const std::string ShowClockKey        { "ShowClock" };
    inline static const std::string FileSaveTimeKey     { "FileSaveTime" };
    inline static const std::string MaxScreenSizeKey    { "MaxScreenSize" };

public:
    inline static const std::string ConfigDir           { "cfg" };
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
    inline static const std::string KeyKey      { "Key" };
    inline static const std::string CmdKey      { "Cmd" };

public:
    CmdMap keyMap;

    bool Load(const path_t& file);
    bool Save(const path_t& file);
};

extern class EditorConfig g_editorConfig;

} //namespace _Editor
