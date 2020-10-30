#pragma once

#ifndef ELPP_THREAD_SAFE
	#define ELPP_THREAD_SAFE
#endif
#include "easylogging++.h"

void ConfigureLogger(const std::string& logFileName, const std::uint64_t maxLogSize, const bool logScreen);