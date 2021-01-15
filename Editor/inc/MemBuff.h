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

#include <cstdint>
#include <array>
#include <list>
#include <memory>
#include <string>


/////////////////////////////////////////////////////////////////////////////
//#define MEM_DEBUG
#ifdef  MEM_DEBUG
  #define BUFF_SIZE       1024
  #define STEP_BLOCKS       10
  #define MAXBLOCKS_NUM     64
  #define STR_NUM          256
  //#define MAX_STRLEN       256
#else
  #define BUFF_SIZE    0x10000 //64k max
  #define STEP_BLOCKS     0x40
  #define MAXBLOCKS_NUM  0x400
  #define STR_NUM       0x1000
  //#define MAX_STRLEN    0x8000
#endif

/////////////////////////////////////////////////////////////////////////////
struct _hbuff
{
    uint64_t    index   : 32;
    uint64_t    version : 32;

    _hbuff(uint64_t i = 0) : index{ i }, version{ 0 } {}
    friend bool operator==(const _hbuff& v1, const _hbuff& v2) { return v1.index == v2.index && v1.version == v2.version; }
    friend bool operator!=(const _hbuff& v1, const _hbuff& v2) { return v1.index != v2.index || v1.version != v2.version; }
};
using hbuff_t = _hbuff;

template <typename Tbuff>
class BuffPool
{
    std::array<std::shared_ptr<Tbuff>, MAXBLOCKS_NUM> m_blockArray;
    //in blocksPool used blocks are in the begin and free blocks are in the end
    std::list<hbuff_t>  m_blockPool;
    size_t              m_usedBlocks{};
    size_t              m_stepBlocks{};

public:
    BuffPool(size_t n = STEP_BLOCKS);
    ~BuffPool() = default;

    size_t      GetBuffSize() const {return BUFF_SIZE;}
    hbuff_t     GetFreeBuff();                            //relink buff to top of pool
    bool        ReleaseBuff(hbuff_t hbuff);               //relink to end of pool
    std::shared_ptr<Tbuff> GetBuffPointer(hbuff_t hbuff); //get buff pointer and del from pool
    bool        ReleaseBuffPointer(hbuff_t hbuff);        //put buff to pool
};

/////////////////////////////////////////////////////////////////////////////
template <typename Tbuff, typename Tview>
class SBuff
{
protected:
    //we use last element as 'end of buffer' offset
    std::array<size_t, STR_NUM + 1> m_strOffsets{};
    size_t                          m_strCount{0};
    bool                            m_mod{false};

    std::shared_ptr<Tbuff>          m_buff;

public:
    SBuff() = default;
    ~SBuff() = default;

    size_t  GetStrCount() {return m_strCount;}
    bool    Clear();
    Tview   GetStr(size_t n);
    bool    AddStr(size_t n, const Tview str);
    bool    AppendStr(const Tview str);
    bool    ChangeStr(size_t n, const Tview str);
    bool    DelStr(size_t n);
};

/////////////////////////////////////////////////////////////////////////////
template <typename Tbuff, typename Tview>
class StrBuff : public SBuff<Tbuff, Tview>
{
    //friend class TextBuff;
    //friend class MStrBuff;

    std::shared_ptr<BuffPool<Tbuff>> m_buffPool;

    //we save string in buffer as in file
    hbuff_t     m_buffHandle{0};
    uint64_t    m_fileOffset{};//offset from begin of file
    bool        m_lostData{false};

public:
    StrBuff(std::shared_ptr<BuffPool<Tbuff>> pool) : m_buffPool{ pool } {};
    ~StrBuff();

    Tview   GetBuff();
    bool    ReleaseBuff();
    bool    Clear();
    bool    ClearModifyFlag();
};
