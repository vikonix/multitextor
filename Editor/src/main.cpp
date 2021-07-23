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
#ifdef USE_VLD
  #include <vld.h>
#endif

#include "utils/logger.h"
#include "utils/Directory.h"
#include "utfcpp/utf8.h"
#include "cxxopts/cxxopts.hpp"
#include "EditorApp.h"
#include "Version.h"
#include "Config.h"


#include <filesystem>

using namespace _Editor;

EditorApp app;
Application& Application::s_app{app};

/////////////////////////////////////////////////////////////////////////////
void PrintKeys()
{
    std::cout << "Editor keyboard combination map to editor commands." << std::endl;
    for (const auto& map : { g_defaultAppKeyMap, g_defaultEditKeyMap })
        for (const auto& [keys, cmd] : map)
        {
            std::string keystr;
            std::string cmdstr;

            for (auto k : keys)
                keystr += (keystr.size() ? "  " : "") + app.GetCodeName(k);
            for (auto c : cmd)
                cmdstr += (cmdstr.size() ? " + " : "") + app.GetCodeName(c);

            std::cout << "Keys: " << std::left << std::setw(16) << keystr << "\t cmd: " << cmdstr << std::endl;
        }
}

void SaveConfig()
{
    //common config dir
    auto configPath = Directory::RunPath() / EditorConfig::ConfigDir;
    std::filesystem::create_directories(configPath);
    g_editorConfig.Save(configPath / EditorConfig::ConfigFile, true);

    KeyConfig keyConfig;
    keyConfig.Save(configPath / g_editorConfig.keyFile);

    //parser dir
    auto parserPath = Directory::RunPath() / ParserConfig::ConfigDir;
    std::filesystem::create_directories(parserPath);
    for (auto& parser : LexParser::s_lexConfig)
    {
        std::string file{ parser.first + ParserConfig::Ext };
        std::replace(file.begin(), file.end(), '+', 'p');
        std::transform(file.begin(), file.end(), file.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); }
        );

        ParserConfig config;
        config.Save(parserPath / file, parser.second);
    }

    //user config dir
    auto userPath = Directory::UserCfgPath(EDITOR_NAME, true);
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) try
{
    auto tmpPath = _Utils::Directory::TmpPath("m");
    auto log = tmpPath / "m-%datetime{%Y%M%d}.log";

    ConfigureLogger(log.u8string());
    LOG(INFO);
    LOG(INFO) << EDITOR_NAME "-" EDITOR_VERSION;

    cxxopts::Options options(EDITOR_NAME, EDITOR_NAME "-" EDITOR_VERSION ". Console mode text editor.");
    options.add_options()
        ("h,help", "Print usage")
        ("k,keys", "Print key map combinations")
        ("c,config", "Save default config files")
        ;

    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        return 0;
    }
    else if (result.count("keys"))
    {
        PrintKeys();
        return 0;
    }
    else if (result.count("config"))
    {
        SaveConfig();
        return 0;
    }

    app.Init();
    app.WriteAppName(EDITOR_NAME);
    app.SetLogo(g_logo);
    if(g_editorConfig.showAccessMenu)
        app.SetAccessMenu(g_accessMenu);
    if(g_editorConfig.showClock)
        app.SetClock(clock_pos::bottom);
    app.SetMenu(g_mainMenu);
    app.SetCmdParser(g_AppKeyMap);
    app.Refresh();

    auto& files = result.unmatched();
    if (!files.empty())
    {
        for (auto& f : files)
        {
            std::filesystem::path path = utf8::utf8to16(f);
            auto [t, parser] = LexParser::GetFileType(path);
            app.OpenFile(f, parser, "UTF-8");
        }
    }
    else
        _TRY(app.LoadSession(std::nullopt));

    app.MainProc(K_EXIT);
    app.Deinit();

    LOG(INFO) << "Exit";
    return 0;
}
catch (const std::exception& ex)
{
    LOG(ERROR) << __FUNC__ << " Exeption: " << ex.what();
    std::cerr << "Exeption: " << ex.what();
    app.Deinit();
    return -1;
}
catch (...)
{
    LOG(ERROR) << __FUNC__ << " Unhandle exeption.";
    std::cerr << "Unhandle exeption";
    app.Deinit();
    return -1;
}
