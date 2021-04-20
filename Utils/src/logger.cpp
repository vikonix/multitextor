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

#include <iomanip>

INITIALIZE_EASYLOGGINGPP

using namespace el;
using namespace el::base;
using namespace el::base::utils;

namespace _Utils
{

void ConfigureLogger(const std::string& logFileName, const std::uint64_t maxLogSize, const bool logScreen)
{
    Configurations cfg;
    //cfg.setGlobally(ConfigurationType::Format, "[%datetime{%Y-%M-%d %H:%m:%s.%g}] [%thread] [%levshort]- %msg");
    cfg.setGlobally(ConfigurationType::Format, "[%datetime{%H:%m:%s.%g}][%thread][%levshort]- %msg");
    cfg.setGlobally(ConfigurationType::Filename, logFileName);
    cfg.setGlobally(ConfigurationType::MaxLogFileSize, std::to_string(maxLogSize));
    cfg.setGlobally(ConfigurationType::ToStandardOutput, logScreen ? "true" : "false");
    cfg.setGlobally(ConfigurationType::PerformanceTracking, "true");
    cfg.setGlobally(ConfigurationType::LogFlushThreshold, "1");
    Loggers::setDefaultConfigurations(cfg, true);

    Loggers::addFlag(LoggingFlag::DisableApplicationAbortOnFatalLog);
    Loggers::addFlag(LoggingFlag::ImmediateFlush);
    Loggers::addFlag(LoggingFlag::StrictLogFileSizeCheck);
    Loggers::addFlag(LoggingFlag::DisableVModulesExtensions);
}

std::string CastEscString(const std::string& string)
{
    std::stringstream ss;

    for (auto ch : string)
    {
        if (ch >= ' ' && ch < 0x7f)
            ss << ch;
        else
            ss << "\\" << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(static_cast<uint8_t>(ch));
    }

    return ss.str();
}

} //namespace _Utils
