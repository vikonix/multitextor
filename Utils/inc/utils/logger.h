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

#ifndef ELPP_THREAD_SAFE
	#define ELPP_THREAD_SAFE
#endif
#include "easyloggingpp/easylogging++.h"

#ifdef _assert
#undef _assert
#endif

#ifdef WIN32
#define __FUNC__ __FUNCTION__ " "
#ifdef _DEBUG
#define _assert(v) _ASSERT(v)
#else
#define _assert(v) if(!(v)){LOG(ERROR) << __FUNC__ << ":" << __LINE__ << " _assert(" << #v << ")";}
#endif
#else
#define __FUNC__ __PRETTY_FUNCTION__ 
#define _assert(v) if(!(v)){LOG(ERROR) << __FUNC__ << ":" << __LINE__ << " _assert(" << #v << ")";}
#endif

#define _TRY(v) {try{v;}catch(const std::exception& ex){LOG(ERROR) << __FUNC__ << ":" << __LINE__  << "-" << ex.what();}} 

namespace _Utils
{

void ConfigureLogger(const std::string& logFileName, const std::uint64_t maxLogSize = 0x1000000, const bool logScreen = false);
std::string CastEscString(const std::string& string);

} //namespace _Utils
