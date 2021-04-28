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
#include "cxxopts/cxxopts.hpp"
#include "EditorApp.h"
#include "Version.h"

using namespace _Editor;

/////////////////////////////////////////////////////////////////////////////
EditorApp app;
Application& Application::s_app{app};

int main(int argc, char** argv) try
{
    ConfigureLogger("m-%datetime{%Y%M%d}.log");
    LOG(INFO);
    LOG(INFO) << EDITOR_NAME "-" EDITOR_VERSION;

    cxxopts::Options options(EDITOR_NAME, EDITOR_NAME "-" EDITOR_VERSION ". Console mode text editor.");
    options.add_options()
        ("h,help", "Print usage")
        ("k,keys", "Print key combinations")
        ;

    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        return 0;
    }
    else if (result.count("keys"))
    {
        for(const auto& [keys, cmd] : g_defaultAppKeyMap)
        {
        }
        for (const auto& [keys, cmd] : g_defaultEditKeyMap)
        {
        }

        return 0;
    }

    app.Init();
    app.SetLogo(g_logo);
    app.WriteAppName(EDITOR_NAME);
    app.SetMenu(g_mainMenu);
    app.SetAccessMenu(g_accessMenu);
    app.SetCmdParser(g_defaultAppKeyMap);
    app.SetClock(clock_pos::bottom);
    app.Refresh();

    auto& files = result.unmatched();
    for (auto& f : files)
    {
        app.OpenFile(f, "Text", "UTF-8");
    }
    app.MainProc(K_EXIT);
    app.Deinit();

    LOG(INFO) << "Exit";
    return 0;
}
catch (...)
{
    LOG(ERROR) << "Unhandle exeption";
}
