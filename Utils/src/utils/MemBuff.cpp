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
#include "utils/MemBuff.h"
#include "utils/logger.h"


/////////////////////////////////////////////////////////////////////////////
template <typename Tbuff>
BuffPool<Tbuff> BuffPool<Tbuff>::s_pool;

template <typename Tbuff>
BuffPool<Tbuff>::BuffPool(size_t n)
{
    m_stepBlocks = n;

    if (n > m_blockArray.size())
        n = m_blockArray.size();

    for (size_t i = 0; i < n; ++i)
    {
        m_blockPool.emplace_back(i);
        ++m_usedBlocks;
    }
}

template <typename Tbuff>
hbuff_t BuffPool<Tbuff>::GetFreeBuff()
{
    if (m_blockPool.empty())
    {
        if (m_usedBlocks >= m_blockArray.size())
        {
            _assert(!"No free blocks");
            return {};
        }

        for (size_t i = 0; i < m_stepBlocks && m_usedBlocks < m_blockArray.size(); ++i)
        {
            m_blockPool.emplace_back(m_usedBlocks);
            ++m_usedBlocks;
        }
    }

    hbuff_t hbuff = m_blockPool.back();
    ++hbuff.version;

    m_blockPool.pop_back();
    m_blockPool.push_front(hbuff);

    return hbuff;
}

template <typename Tbuff>
bool BuffPool<Tbuff>::ReleaseBuff(hbuff_t hbuff)
{
    if (hbuff.index >= m_usedBlocks)
        return false;

    for (auto it = m_blockPool.cbegin(); it != m_blockPool.cend(); ++it)
    {
        if (*it == hbuff)
        {
            m_blockPool.erase(it);
            m_blockPool.push_front(hbuff);
            break;
        }
    }

    return true;
}

template <typename Tbuff>
std::shared_ptr<Tbuff> BuffPool<Tbuff>::GetBuffPointer(hbuff_t hbuff)
{
    if (hbuff.index >= m_usedBlocks)
        return nullptr;

    bool found{false};
    for (auto it = m_blockPool.cbegin(); it != m_blockPool.cend(); ++it)
    {
        if (*it == hbuff)
        {
            m_blockPool.erase(it);
            found = true;
            break;
        }
    }
    if (!found)
        //block was lost
        return nullptr;

    auto& ptr = m_blockArray[hbuff.index];
    if (!ptr)
        ptr = std::make_shared<Tbuff>();
    if (ptr)
        ptr->reserve(BUFF_SIZE);
    return ptr;
}

template <typename Tbuff>
bool BuffPool<Tbuff>::ReleaseBuffPointer(hbuff_t hbuff)
{
    if (hbuff.index >= m_usedBlocks)
        return false;

    m_blockPool.push_front(hbuff);
    return true;
}

/////////////////////////////////////////////////////////////////////////////
template <typename Tbuff, typename Tview>
bool SBuff<Tbuff, Tview>::Clear()
{
    m_strOffsetList.clear();
    m_mod = false;
    return true;
}

template <typename Tbuff, typename Tview>
Tview SBuff<Tbuff, Tview>::GetStr(size_t n)
{
    if (!m_buff)
        return {};

    if(m_strOffsetList.empty() || n >= GetStrCount())
        return {};

    auto begin = GetStrOffset(n);
    auto end = GetStrOffset(n + 1);

    Tview view(m_buff->c_str() + begin, end - begin);
    return view;
}

template <typename Tbuff, typename Tview>
bool SBuff<Tbuff, Tview>::AppendStr(const Tview str)
{
    if (!m_buff)
        return false;

    auto offset_end = GetBuffSize();
    uint32_t dl = static_cast<uint32_t>(str.size());

    if (offset_end + dl > m_buff->capacity())
        return false;

    m_buff->append(str);

    m_strOffsetList.push_back(offset_end + dl);

    m_mod = true;
    return true;
}

template <typename Tbuff, typename Tview>
bool SBuff<Tbuff, Tview>::AddStr(size_t n, const Tview str)
{
    if (!m_buff)
        return false;

    if (n > GetStrCount())
        return false;

    auto offset_n = GetStrOffset(n);
    auto offset_end = GetBuffSize();
    uint32_t dl = static_cast<uint32_t>(str.size());

    if (offset_end + dl > m_buff->capacity())
        return false;

    m_buff->insert(offset_n, str);

    m_strOffsetList.push_back(0);
    size_t i;
    for (i = GetStrCount() - 1; i > n; --i)
        m_strOffsetList[i] = m_strOffsetList[i - 1] + dl;
    if (0 != i)
        m_strOffsetList[i] = m_strOffsetList[i - 1] + dl;
    else
        m_strOffsetList[i] = dl;

    m_mod = true;
    return true;
}

template <typename Tbuff, typename Tview>
bool SBuff<Tbuff, Tview>::ChangeStr(size_t n, const Tview str)
{
    if (!m_buff)
        return false;

    if (n >= GetStrCount())
        return false;

    auto offset_n = GetStrOffset(n);
    auto offset_n1 = GetStrOffset(n + 1);
    auto offset_end = GetBuffSize();
    int dl = static_cast<int>(str.size()) - static_cast<int>(offset_n1 - offset_n);

    if(dl > 0 && static_cast<size_t>(offset_end) + dl > m_buff->capacity())
        return false;

    m_buff->replace(offset_n, offset_n1 - offset_n, str);

    for (size_t i = n; i < GetStrCount(); ++i)
        m_strOffsetList[i] += dl;

    m_mod = true;
    return true;
}

template <typename Tbuff, typename Tview>
bool SBuff<Tbuff, Tview>::DelStr(size_t n)
{
    if (!m_buff)
        return false;

    if (n >= GetStrCount())
        return false;

    auto offset_n = GetStrOffset(n);
    auto offset_n1 = GetStrOffset(n + 1);
    auto dl = offset_n1 - offset_n;

    m_buff->erase(offset_n, dl);

    for (size_t i = n; i < GetStrCount() - 1; ++i)
        m_strOffsetList[i] = m_strOffsetList[i + 1] - dl;
    m_strOffsetList.pop_back();

    m_mod = true;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
template <typename Tbuff, typename Tview>
StrBuff<Tbuff, Tview>::~StrBuff()
{
    if (SBuff<Tbuff, Tview>::m_buff)
    {
        BuffPool<Tbuff>::s_pool.ReleaseBuffPointer(m_buffHandle);
        SBuff<Tbuff, Tview>::m_buff = nullptr;
    }
    if (m_buffHandle != 0)
    {
        BuffPool<Tbuff>::s_pool.ReleaseBuff(m_buffHandle);
    }
}

template <typename Tbuff, typename Tview>
std::shared_ptr<Tbuff> StrBuff<Tbuff, Tview>::GetBuff()
{
    if (m_buffHandle == 0)
        //we have no block
        m_buffHandle = BuffPool<Tbuff>::s_pool.GetFreeBuff();

    if (!SBuff<Tbuff, Tview>::m_buff)
        //we have no pointer
        SBuff<Tbuff, Tview>::m_buff = BuffPool<Tbuff>::s_pool.GetBuffPointer(m_buffHandle);

    if (!SBuff<Tbuff, Tview>::m_buff)
    {
        //we lost buffer
        m_buffHandle = BuffPool<Tbuff>::s_pool.GetFreeBuff();
        if (m_buffHandle != 0)
        {
            SBuff<Tbuff, Tview>::m_buff = BuffPool<Tbuff>::s_pool.GetBuffPointer(m_buffHandle);
            m_lostData = true;
        }
    }

    return SBuff<Tbuff, Tview>::m_buff;
}

template <typename Tbuff, typename Tview>
bool StrBuff<Tbuff, Tview>::ReleaseBuff()
{
    bool rc = true;
    if (SBuff<Tbuff, Tview>::m_buff && !SBuff<Tbuff, Tview>::m_mod)
    {
        rc = BuffPool<Tbuff>::s_pool.ReleaseBuffPointer(m_buffHandle);
        SBuff<Tbuff, Tview>::m_buff = nullptr;
    }
    return rc;
}

template <typename Tbuff, typename Tview>
bool StrBuff<Tbuff, Tview>::Clear()
{
    bool rc = true;
    if (SBuff<Tbuff, Tview>::m_buff)
        rc = BuffPool<Tbuff>::s_pool.ReleaseBuffPointer(m_buffHandle);
    if (m_buffHandle != 0)
        rc = BuffPool<Tbuff>::s_pool.ReleaseBuff(m_buffHandle);

    m_buffHandle = 0;
    SBuff<Tbuff, Tview>::m_buff = nullptr;

    m_fileOffset = 0;
    m_lostData = false;

    return rc;
}

template <typename Tbuff, typename Tview>
bool StrBuff<Tbuff, Tview>::ClearModifyFlag()
{
    bool rc = true;
    SBuff<Tbuff, Tview>::m_mod = false;
    if (SBuff<Tbuff, Tview>::m_buff)
    {
        rc = BuffPool<Tbuff>::s_pool.ReleaseBuffPointer(m_buffHandle);
        SBuff<Tbuff, Tview>::m_buff = nullptr;
    }
    return rc;
}

/////////////////////////////////////////////////////////////////////////////
template <typename Tbuff, typename Tview>
MemStrBuff<Tbuff, Tview>::MemStrBuff()
{
    m_curBuff = m_buffList.end();
}

template <typename Tbuff, typename Tview>
std::optional<typename std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>>::iterator> MemStrBuff<Tbuff, Tview>::GetBuff(size_t& line)
{
    if (m_buffList.empty())
    {
        auto newBuff = std::make_shared<StrBuff<Tbuff, Tview>>();
        m_buffList.push_back(newBuff);
        m_curBuff = m_buffList.begin();
    }

    if (line > m_totalStrCount)
        return std::nullopt;

    if (m_curBuff == m_buffList.end() && m_curBuffLine == 0)
    {
        m_curBuff = m_buffList.begin();
        m_curBuffLine = 0;
    }

    if (line < m_curBuffLine)
    {
        //LOG(DEBUG) << "GetBuff prev l=" << line;
        while (m_curBuff != m_buffList.begin())
        {
            --m_curBuff;
            //m_curBuffLine -= (*m_curBuff)->m_strCount;
            m_curBuffLine -= (*m_curBuff)->GetStrCount();
            if (line > m_curBuffLine)
                break;
        }
    }
    else if (line >= m_curBuffLine + (*m_curBuff)->GetStrCount())
    {
        //LOG(DEBUG) << "GetBuff next l=" << line;
        while (m_curBuff != m_buffList.end())
        {
            auto count = (*m_curBuff)->GetStrCount();
            ++m_curBuff;
            if (m_curBuff == m_buffList.end())
            {
                --m_curBuff;
                break;
            }
            m_curBuffLine += count;
            if (line < m_curBuffLine + (*m_curBuff)->GetStrCount())
                break;
        }
    }

    auto blockBuff = (*m_curBuff)->GetBuff();
    if (!blockBuff)
    {
        _assert(!"no memory");
        return std::nullopt;
    }

    if ((*m_curBuff)->m_lostData)
    {
        //LOG(DEBUG) << "curBuff->m_lostData first=" << m_curBuffLine << " last=" << m_curBuffLine + (*m_curBuff)->GetStrCount() - 1;

        bool rc = LoadBuff((*m_curBuff)->m_fileOffset, (*m_curBuff)->GetBuffSize(), (*m_curBuff)->GetBuff());
        if (!rc)
            return std::nullopt;

        (*m_curBuff)->m_lostData = false;
    }

    line -= m_curBuffLine;
    //don't forgot to call release buffer in external function after buffer using
    //m_curBuff->ReleaseBuff();

    return m_curBuff;
}

template <typename Tbuff, typename Tview>
std::shared_ptr<StrBuff<Tbuff, Tview>> MemStrBuff<Tbuff, Tview>::GetNewBuff()
{ 
    auto newBuff = std::make_shared<StrBuff<Tbuff, Tview>>();
    m_buffList.push_back(newBuff);
    return newBuff;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::ReleaseBuff()
{
    if (m_curBuff != m_buffList.end())
    {
        (*m_curBuff)->ReleaseBuff();
    }
    return true;
}

template <typename Tbuff, typename Tview>
size_t MemStrBuff<Tbuff, Tview>::GetSize() const
{
    size_t size = 0;

    for (const auto& buff: m_buffList)
    {
        size += buff->GetBuffSize();
    }

    return size;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::Clear()
{
    m_buffList.clear();
    m_curBuff = m_buffList.end();
    m_totalStrCount = 0;
    m_changed = false;
    m_curBuffLine = 0;

    return true;
}

template <typename Tbuff, typename Tview>
Tview MemStrBuff<Tbuff, Tview>::GetStr(size_t n)
{
    if (n >= m_totalStrCount)
        return {};

    auto buff = GetBuff(n);
    if (!buff || n > (**buff)->GetStrCount())
        return {};

    return (**buff)->GetStr(n);
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::SplitBuff(typename std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>>::iterator buff, size_t line)
{
    _assert(buff != m_buffList.end());

    auto oldBuff = *buff;
    size_t offset = oldBuff->GetStrOffset(line);
    size_t split = 0;

    size_t limit;
    if (offset < BUFF_SIZE / 3)
        // 1/3
        limit = BUFF_SIZE / 3;
    else if (offset < (BUFF_SIZE / 3) * 2)
        // 1/2
        limit = BUFF_SIZE / 2;
    else
        // 2/3
        limit = (BUFF_SIZE / 3) * 2;

    for (split = 0; oldBuff->GetStrOffset(split) < limit; ++split);

    auto newBuff = std::make_shared<StrBuff<Tbuff, Tview>>();

    auto oldBuffData = oldBuff->GetBuff();
    auto newBuffData = newBuff->GetBuff();
    if (!oldBuffData || !newBuffData)
    {
        oldBuff->ReleaseBuff();
        newBuff->ReleaseBuff();

        LOG(ERROR) << "ERROR GetBuff";
        return false;
    }

    newBuff->m_mod = true;
    oldBuff->m_mod = true;

    uint32_t begin = oldBuff->GetStrOffset(split);
    uint32_t end = oldBuff->GetBuffSize();

    newBuffData->resize(end - begin);
    memcpy(newBuffData->data(), oldBuffData->c_str() + begin, end - begin);

    for (size_t i = 1; i + split <= oldBuff->GetStrCount(); ++i)
        newBuff->m_strOffsetList.push_back((oldBuff->GetStrOffset(i + split) - begin));

    oldBuff->m_strOffsetList.erase(oldBuff->m_strOffsetList.begin() + split, oldBuff->m_strOffsetList.end());
    oldBuffData->resize(begin);

    m_buffList.insert(++buff, newBuff);

    //LOG(DEBUG) << "n=" << oldBuff->m_strCount << " old=" << split << " new=" << newBuff->m_strCount;

    if (line < split)
    {
        LOG(DEBUG) << "SplitBuff at n=" << split << " use old buff";
        newBuff->ReleaseBuff();
    }
    else
    {
        LOG(DEBUG) << "SplitBuff at n=" << split << " use new buff";
        oldBuff->ReleaseBuff();
    }
    
    return true;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::DelBuff(typename std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>>::iterator& buff)
{
    //TPRINT(("DelBuff\n"));
    if (buff == m_curBuff)
        m_curBuff = m_buffList.end();

    m_buffList.erase(buff);
    return true;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::AddStr(size_t n, const Tview str)
{
    if (n > m_totalStrCount)
        return false;

    size_t _n = n;

    //LOG(DEBUG) << "AddStr n=" << n << " '" << str << "'";

    auto buff = GetBuff(n);
    if (!buff)
    {
        _assert(0);
        return false;
    }
        
    bool rc = (**buff)->AddStr(n, str);
    if (!rc)
    {
        if (n == (**buff)->GetStrCount())
        {
            //LOG(DEBUG) << "Last line " << _n << ". Create new buff=" << m_buffList.size();
            auto newBuff = std::make_shared<StrBuff<Tbuff, Tview>>();
            m_buffList.push_back(newBuff);
            n = _n;
            buff = GetBuff(n);
            if (!buff)
            {
                _assert(0);
                return false;
            }

            rc = (**buff)->AddStr(n, str);
        }
        else
        {
            rc = SplitBuff(*buff, n);
            if (rc)
            {
                n = _n;
                buff = GetBuff(n);
                if (!buff)
                {
                    _assert(0);
                    return false;
                }
                rc = (**buff)->AddStr(n, str);
            }
        }
    }

    if (rc)
        ++m_totalStrCount;

    if (buff)
        (**buff)->ReleaseBuff();

    m_changed = true;

    //LOG(DEBUG) << "rc=" << rc << " strcount=" << m_strCount;
    return rc;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::ChangeStr(size_t n, const Tview str)
{
    //LOG(DEBUG) << "ChangeStr " << n;
    if (n >= m_totalStrCount)
        return false;

    auto buff = GetBuff(n);
    if (!buff)
        return false;
    size_t _n = n;

    bool rc = (**buff)->ChangeStr(n, str);
    if (!rc)
    {
        rc = SplitBuff(*buff, n);
        if (!rc)
        {
            n = _n;
            buff = GetBuff(n);
            if (!buff)
                rc = false;
            else
                rc = (**buff)->ChangeStr(n, str);
        }
        if (!rc)
        {
            LOG(ERROR) << __FUNC__;
            _assert(0);
        }
    }

    if (buff)
        (**buff)->ReleaseBuff();

    m_changed = true;

    return rc;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::DelStr(size_t n)
{
    //LOG(DEBUG) << "DelStr " << n;

    auto buff = GetBuff(n);
    if (!buff)
        return false;

    bool rc = (**buff)->DelStr(n);
    if (!rc)
    {
        _assert(0);
        (**buff)->ReleaseBuff();
        return false;
    }
    
    --m_totalStrCount;
    m_changed = true;

    if ((**buff)->m_strOffsetList.empty())
    {
        LOG(DEBUG) << "EmptyBlock";
        DelBuff(*buff);
    }
    else
        (**buff)->ReleaseBuff();

    return true;
}

template class BuffPool<std::string>;
template class SBuff<std::string, std::string_view>;
template class StrBuff<std::string, std::string_view>;
template class MemStrBuff<std::string, std::string_view>;
