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
#include "MemBuff.h"
#include "utils/logger.h"


template class BuffPool<std::string>;
template class SBuff<std::string, std::string_view>;
template class StrBuff<std::string, std::string_view>;

/////////////////////////////////////////////////////////////////////////////
template <typename Tbuff>
BuffPool<Tbuff>::BuffPool(size_t n)
{
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
            m_blockPool.push_back(hbuff);
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
    m_strOffsets.fill(0);
    m_strCount = 0;
    m_mod = false;
    return true;
}

template <typename Tbuff, typename Tview>
Tview SBuff<Tbuff, Tview>::GetStr(size_t n)
{
    if (!m_buff)
        return {};

    if(!m_strCount || n >= m_strCount)
        return {};

    size_t begin = m_strOffsets[n];
    size_t end   = m_strOffsets[n + 1];

    Tview view(m_buff->c_str() + begin, end - begin);
    return view;
}

template <typename Tbuff, typename Tview>
bool SBuff<Tbuff, Tview>::AddStr(size_t n, const Tview str)
{
    if (!m_buff)
        return false;

    if (m_strCount == STR_NUM)
        return false;

    if (n > m_strCount)
        return false;

    size_t offset_n = m_strOffsets[n];
    size_t offset_end = m_strOffsets[m_strCount];
    size_t dl = str.size();

    if (offset_end + str.size() > m_buff->capacity())
        return false;

    m_buff->insert(offset_n, str);

    ++m_strCount;
    for (size_t i = m_strCount; i > n; --i)
        m_strOffsets[i] = m_strOffsets[i - 1] + dl;

    m_mod = true;
    return true;
}

template <typename Tbuff, typename Tview>
bool SBuff<Tbuff, Tview>::AppendStr(const Tview str)
{
    if (!m_buff)
        return false;

    if (m_strCount == STR_NUM)
        return false;

    size_t offset_end = m_strOffsets[m_strCount];
    size_t dl = str.size();

    if (offset_end + str.size() > m_buff->capacity())
        return false;

    m_buff->append(str);

    ++m_strCount;
    m_strOffsets[m_strCount] = offset_end + dl;

    m_mod = true;
    return true;
}

template <typename Tbuff, typename Tview>
bool SBuff<Tbuff, Tview>::ChangeStr(size_t n, const Tview str)
{
    if (!m_buff)
        return false;

    if (n > m_strCount)
        return false;

    size_t offset_n = m_strOffsets[n];
    size_t offset_n1 = m_strOffsets[n + 1];
    size_t offset_end = m_strOffsets[m_strCount];
    int dl = (int)str.size() - (int)(offset_n1 - offset_n);

    if(dl > 0 && offset_end + dl > m_buff->capacity())
        return false;

    m_buff->replace(offset_n, offset_n1, str);

    for (size_t i = n + 1; i <= m_strCount; ++i)
        m_strOffsets[i] += dl;

    m_mod = true;
    return true;
}

template <typename Tbuff, typename Tview>
bool SBuff<Tbuff, Tview>::DelStr(size_t n)
{
    if (!m_buff)
        return false;

    if (n > m_strCount)
        return false;

    size_t offset_n = m_strOffsets[n];
    size_t offset_n1 = m_strOffsets[n + 1];
    size_t dl = offset_n1 - offset_n;

    m_buff->erase(offset_n, dl);

    for (size_t i = n + 1; i < m_strCount; ++i)
        m_strOffsets[i] = m_strOffsets[i + 1] - dl;
    --m_strCount;

    m_mod = true;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
template <typename Tbuff, typename Tview>
StrBuff<Tbuff, Tview>::~StrBuff()
{
    if (SBuff<Tbuff, Tview>::m_buff)
    {
        m_buffPool->ReleaseBuffPointer(m_buffHandle);
        SBuff<Tbuff, Tview>::m_buff = nullptr;
    }
    if (m_buffHandle != 0)
    {
        m_buffPool->ReleaseBuff(m_buffHandle);
    }
}

template <typename Tbuff, typename Tview>
Tview StrBuff<Tbuff, Tview>::GetBuff()
{
    if (m_buffHandle == 0)
        //we have no block
        m_buffHandle = m_buffPool->GetFreeBuff();

    if (!SBuff<Tbuff, Tview>::m_buff)
        //we have no pointer
        SBuff<Tbuff, Tview>::m_buff = m_buffPool->GetBuffPointer(m_buffHandle);

    if (!SBuff<Tbuff, Tview>::m_buff)
    {
        //we lost buffer
        m_buffHandle = m_buffPool->GetFreeBuff();
        if (m_buffHandle != 0)
        {
            SBuff<Tbuff, Tview>::m_buff = m_buffPool->GetBuffPointer(m_buffHandle);
            m_lostData = true;
        }
    }

    return SBuff<Tbuff, Tview>::m_buff->c_str();
}

template <typename Tbuff, typename Tview>
bool StrBuff<Tbuff, Tview>::ReleaseBuff()
{
    bool rc = true;
    if (SBuff<Tbuff, Tview>::m_buff && !SBuff<Tbuff, Tview>::m_mod)
    {
        rc = m_buffPool->ReleaseBuffPointer(m_buffHandle);
        SBuff<Tbuff, Tview>::m_buff = nullptr;
    }
    return rc;
}

template <typename Tbuff, typename Tview>
bool StrBuff<Tbuff, Tview>::Clear()
{
    bool rc = true;
    if (SBuff<Tbuff, Tview>::m_buff)
        rc = m_buffPool->ReleaseBuffPointer(m_buffHandle);
    if (m_buffHandle != 0)
        rc = m_buffPool->ReleaseBuff(m_buffHandle);

    m_buffHandle = 0;
    SBuff<Tbuff, Tview>::m_buff = nullptr;

    m_fileOffset = 0;
    m_lostData = false;

    SBuff<Tbuff, Tview>::Clear();
    return rc;
}

template <typename Tbuff, typename Tview>
bool StrBuff<Tbuff, Tview>::ClearModifyFlag()
{
    bool rc = true;
    SBuff<Tbuff, Tview>::m_mod = false;
    if (SBuff<Tbuff, Tview>::m_buff)
    {
        rc = m_buffPool->ReleaseBuffPointer(m_buffHandle);
        SBuff<Tbuff, Tview>::m_buff = nullptr;
    }
    return rc;
}

