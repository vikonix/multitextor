/*
FreeBSD License

Copyright (c) 2020-2023 vikonix: valeriy.kovalev.software@gmail.com
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
#include <vector>
#include <optional>
#include <functional>


/////////////////////////////////////////////////////////////////////////////
//#define MEM_DEBUG
#ifdef  MEM_DEBUG
  #define BUFF_SIZE      0x4000
  #define STEP_BLOCKS      0x10
  #define MAXBLOCKS_NUM   0x100
#else
  #define BUFF_SIZE     0x10000 //64k max
  #define STEP_BLOCKS     0x100
  #define MAXBLOCKS_NUM  0x1000
#endif

#define MAX_STRLEN (BUFF_SIZE / 2)

namespace _Editor
{
class Editor;
}

/////////////////////////////////////////////////////////////////////////////
namespace _Utils
{

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
    size_t              m_stepBlocks{1};

public:
    static BuffPool     s_pool;

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
class MemStrBuff;

template <typename Tbuff, typename Tview>
class SBuff
{
    friend class MemStrBuff<std::string, std::string_view>;

protected:
    //we use last element as 'end of buffer' offset
    std::vector<uint32_t>           m_strOffsetList{};
    bool                            m_mod{false};
    std::shared_ptr<Tbuff>          m_buff;

    uint32_t GetStrOffset(size_t n) { return n == 0 ? 0 : m_strOffsetList[n - 1]; }

public:
    SBuff() = default;
    ~SBuff() = default;

    bool    Clear();
    size_t  GetStrCount() { return m_strOffsetList.size(); }
    uint32_t GetBuffSize() { return m_strOffsetList.empty() ? 0 : m_strOffsetList.back(); }

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
    friend class MemStrBuff<std::string, std::string_view>;
    friend class _Editor::Editor;

    //we save string in buffer as in file
    hbuff_t     m_buffHandle{0};
    uint64_t    m_fileOffset{};//offset from begin of file
    bool        m_lostData{false};

public:
    StrBuff() = default;
    ~StrBuff();

    std::shared_ptr<Tbuff> GetBuff();
    bool    ReleaseBuff();
    bool    Clear();
    bool    ClearModifyFlag();
};

/////////////////////////////////////////////////////////////////////////////
template <typename Tbuff, typename Tview>
class MemStrBuff
{
    using LoadBuffFunc = std::function<bool(uint64_t offset, size_t size, std::shared_ptr<Tbuff> buff)>;
    friend class _Editor::Editor;

protected:
    LoadBuffFunc    m_loadBuffFunc;
    std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>> m_buffList;
    size_t  m_totalStrCount{};
    bool    m_changed{};

    typename std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>>::iterator m_curBuff;
    size_t  m_curBuffLine{};

    bool LoadBuff(uint64_t offset, size_t size, std::shared_ptr<Tbuff> buff)
    {
        if (m_loadBuffFunc)
            return m_loadBuffFunc(offset, size, buff);
        return true;
    }

    std::optional<typename std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>>::iterator> GetBuff(size_t& line);
    bool    ReleaseBuff();
    bool    SplitBuff(typename std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>>::iterator buff, size_t line);
    bool    DelBuff(typename std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>>::iterator& buff);

public:
    MemStrBuff();

    bool    SetLoadBuffFunc(LoadBuffFunc func) { m_loadBuffFunc = func; return true; };
    bool    IsChanged() const { return m_changed; }
    size_t  GetSize() const;

    bool    Clear();
    bool    ClearModifyFlag();
    size_t  GetStrCount() const { return m_totalStrCount; }
    Tview   GetStr(size_t n);
    bool    AddStr(size_t n, const Tview str);
    bool    AppendStr(const Tview str) {return AddStr(m_totalStrCount, str);}
    bool    ChangeStr(size_t n, const Tview str);
    bool    DelStr(size_t n);

    std::shared_ptr<StrBuff<Tbuff, Tview>> GetNewBuff();

    //std::pair<size_t, bool> FindStr(const std::string& str);
};

} //namespace _Utils
