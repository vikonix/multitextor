#include "logger.h"

INITIALIZE_EASYLOGGINGPP

using namespace el;
using namespace el::base;
using namespace el::base::utils;

void ConfigureLogger(const std::string& logFileName, const std::uint64_t maxLogSize, const bool logScreen)
{
    Configurations cfg;
    //cfg.setGlobally(ConfigurationType::Format, "[%datetime{%Y-%M-%d %H:%m:%s.%g}] [%thread] [%levshort]- %msg");
    cfg.setGlobally(ConfigurationType::Format, "[%datetime{%H:%m:%s.%g}][%thread][%levshort]- %msg");
    cfg.setGlobally(ConfigurationType::Filename, logFileName);
    cfg.setGlobally(ConfigurationType::MaxLogFileSize, std::to_string(maxLogSize)); // 2MB
    cfg.setGlobally(ConfigurationType::ToStandardOutput, logScreen ? "true" : "false");
    cfg.setGlobally(ConfigurationType::PerformanceTracking, "true");
    cfg.setGlobally(ConfigurationType::LogFlushThreshold, "1");
    Loggers::setDefaultConfigurations(cfg, true);

    Loggers::addFlag(LoggingFlag::DisableApplicationAbortOnFatalLog);
    Loggers::addFlag(LoggingFlag::ImmediateFlush);
    Loggers::addFlag(LoggingFlag::StrictLogFileSizeCheck);
    Loggers::addFlag(LoggingFlag::DisableVModulesExtensions);
}

