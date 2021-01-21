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
#include "utils/MemBuff.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////

void CheckDirectoryFunc()
{
    LOG(DEBUG) << "Test: " << __FUNC__;

    LOG(DEBUG) << "run path=" << Directory::RunPath();
    LOG(DEBUG) << "cur path=" << Directory::CurPath();
    LOG(DEBUG) << "tmp path=" << Directory::TmpPath();
    LOG(DEBUG) << "cfg path=" << Directory::CfgPath();
    LOG(DEBUG) << "sys cfg path=" << Directory::SysCfgPath();
    LOG(DEBUG) << "user=" << Directory::UserName();

    _assert( Directory::Match<std::string>("geeks", "g*ks")); // Yes 
    _assert( Directory::Match<std::string>("geeksforgeeks", "ge?ks*")); // Yes 
    _assert(!Directory::Match<std::string>("gee", "g*k"));  // No because 'k' is not in second 
    _assert(!Directory::Match<std::string>("pqrst", "*pqrs")); // No because 't' is not in first 
    _assert( Directory::Match<std::string>("abcdhghgbcd", "abc*bcd")); // Yes 
    _assert(!Directory::Match<std::string>("abcd", "abc*c?d")); // No because second must have 2 instances of 'c' 
    _assert( Directory::Match<std::string>("abcd", "*c*d")); // Yes 
    _assert( Directory::Match<std::string>("abcd", "*?c*d")); // Yes 
    _assert( Directory::Match<std::string>("acd", "*?c*d")); // Yes 
    _assert( Directory::Match<std::string>("abcd", "*?c*d")); // Yes 
    _assert( Directory::Match<std::u16string>(u"abcd", u"*?c*d")); // Yes 
}

void BuffTest()
{
    LOG(DEBUG) << "Test: " << __FUNC__;
    {
        auto pool = std::make_shared<BuffPool<std::string>>();
        auto b = pool->GetFreeBuff();
        auto ptr = pool->GetBuffPointer(b);
        pool->ReleaseBuffPointer(b);
        pool->ReleaseBuff(b);
    }
    {
        auto sbuff = std::make_unique<StrBuff<std::string, std::string_view>>();
        sbuff->GetBuff();

        sbuff->AppendStr("Hello");
        sbuff->AppendStr(" world");
        sbuff->AddStr(1, " our ");
        sbuff->AddStr(1, " my ");
        sbuff->ChangeStr(0, "!!!Hello!!!");
        sbuff->DelStr(1);

        std::stringstream sstr;
        sstr << sbuff->GetStr(0) << sbuff->GetStr(1) << sbuff->GetStr(2);

        LOG(DEBUG) << sstr.str();
        LOG(DEBUG) << *(sbuff->GetBuff());
        _assert(sstr.str() == *(sbuff->GetBuff()));
    }
    {
        auto genStr = [](int i) ->std::string {
            std::stringstream sstr;
            sstr << "str" << i << std::endl;
            return sstr.str();
        };

        int n = 100000;
        LOG(DEBUG) << "gen str " << n;
        MemStrBuff<std::string, std::string_view> mbuff;
        for (int i = 0; i < n; ++i)
        {
            mbuff.AddStr(0, genStr(i));
        }
        LOG(DEBUG) << "check";
        for (int i = 0; i < n; ++i)
        {
            [[maybe_unused]]auto str = mbuff.GetStr(n - i - 1);
            _assert(str == genStr(i));
        }
        LOG(DEBUG) << "ok";
    }
}


int main()
{
    ConfigureLogger("m-%datetime{%Y%M%d}.log", 0x200000, false);
    LOG(INFO);
    LOG(INFO) << "Utils test";

    BuffTest();
    CheckDirectoryFunc();

    LOG(INFO) << "End";

    return 0;
}