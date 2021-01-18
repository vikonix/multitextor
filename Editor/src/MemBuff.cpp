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


/////////////////////////////////////////////////////////////////////////////
template <typename Tbuff>
BuffPool<Tbuff> BuffPool<Tbuff>::s_pool;

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

template <typename Tbuff, typename Tview>
std::optional<typename std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>>::iterator> MemStrBuff<Tbuff, Tview>::GetBuff(size_t& line)
{
    if (line > m_strCount)
        return std::nullopt;

    if (m_curBuff == m_buffList.end())
        m_curBuff = m_buffList.begin();

    if (line < m_curBuffLine)
    {
        LOG(DEBUG) << "GetBuff prev l=" << line;
        while (m_curBuff != m_buffList.begin())
        {
            --m_curBuff;
            m_curBuffLine -= (*m_curBuff)->m_strCount;
            if (line > m_curBuffLine)
                break;
        }
    }
    else if (line >= m_curBuffLine + (*m_curBuff)->m_strCount)
    {
        LOG(DEBUG) << "GetBuff next l=" << line;
        while (m_curBuff != m_buffList.end())
        {
            m_curBuffLine += (*m_curBuff)->m_strCount;
            ++m_curBuff;
            if (line < m_curBuffLine + (*m_curBuff)->m_strCount)
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
        LOG(DEBUG) << "curBuff->m_lostData first=" << m_curBuffLine << " last=" << m_curBuffLine + (*m_curBuff)->m_strCount - 1;

        bool rc = LoadBuff((*m_curBuff)->m_fileOffset, (*m_curBuff)->m_strOffsets[(*m_curBuff)->m_strCount], (*m_curBuff)->GetBuff());
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
bool MemStrBuff<Tbuff, Tview>::ReleaseBuff()
{
    if (m_curBuff != m_buffList.end())
    {
        (*m_curBuff)->ReleaseBuff();
        m_curBuff = m_buffList.end();
    }
    return true;
}

template <typename Tbuff, typename Tview>
size_t MemStrBuff<Tbuff, Tview>::GetSize()
{
    size_t size = 0;

    for (const auto& buff: m_buffList)
    {
        size += buff->m_strOffsets[buff->m_strCount];
    }

    return size;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::Clear()
{
    m_buffList.clear();
    m_curBuff = m_buffList.end();
    m_strCount = 0;
    m_changed = false;
    m_curBuffLine = 0;
    
    return true;
}

template <typename Tbuff, typename Tview>
Tview MemStrBuff<Tbuff, Tview>::GetStr(size_t n)
{
    if (n >= m_strCount)
        return {};

    auto buff = GetBuff(n);
    if (!buff || n > (*buff.value())->m_strCount)
        return {};

    return (*buff.value())->GetStr(n);
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::SplitBuff(typename std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>>::iterator& buff, size_t line)
{
    LOG(DEBUG) << "SplitBuff n=" << line;
    size_t offset = (*buff)->m_strOffsets[line];
    size_t split = 0;

    if ((*buff)->m_strCount == STR_NUM)
    {
        //all string filled
        if (line < STR_NUM / 3)
            // 1/3
            split = STR_NUM / 3;
        else if (line < (STR_NUM / 3) * 2)
            // 1/2
            split = STR_NUM / 2;
        else
            // 2/3
            split = (STR_NUM / 3) * 2;

        if ((*buff)->m_strOffsets[split] > (BUFF_SIZE / 3) * 2)
            //long strings
            // 2/3
            for (split = 0; (*buff)->m_strOffsets[split] < (BUFF_SIZE / 3) * 2; ++split);
    }
    else
    {
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

        for (split = 0; (*buff)->m_strOffsets[split] < limit; ++split);
    }

    auto newBuff = std::make_shared<StrBuff<Tbuff, Tview>>();
    
    auto buffData = (*buff)->GetBuff();
    auto newBuffData = newBuff->GetBuff();
    if (!buffData || !newBuffData)
    {
        (*buff)->ReleaseBuff();
        newBuff->ReleaseBuff();

        LOG(ERROR) << "ERROR GetBuff";
        return false;
    }

    (*buff)->m_mod = true;
    newBuff->m_mod = true;

    size_t begin = (*buff)->m_strOffsets[split];
    size_t end = (*buff)->m_strOffsets[(*buff)->m_strCount];

    newBuffData->resize(end - begin);
    memcpy(newBuffData->data(), buffData->c_str() + begin, end - begin);

    for (size_t i = 0; i + split <= (*buff)->m_strCount; ++i)
        newBuff->m_strOffsets[i] = (*buff)->m_strOffsets[i + split] - begin;

    newBuff->m_strCount = (*buff)->m_strCount - split;
    (*buff)->m_strCount = split;

    m_buffList.insert(buff, newBuff);
    LOG(DEBUG) << "n=" << (*buff)->m_strCount << " n1=" << split << " n2=" << newBuff->m_strCount; 

    (*buff)->ReleaseBuff();
    newBuff->ReleaseBuff();

    if (line < split)
        LOG(DEBUG) << "SplitBuff at n=" << split << " same buff";
    else
        LOG(DEBUG) << "SplitBuff at n=" << split << " new buff";
    
    return true;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::DelBuff(typename std::list<std::shared_ptr<StrBuff<Tbuff, Tview>>>::iterator& buff)
{
    return true;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::AddStr(size_t n, const Tview str)
{
    return true;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::ChangeStr(size_t n, const Tview str)
{
    return true;
}

template <typename Tbuff, typename Tview>
bool MemStrBuff<Tbuff, Tview>::DelStr(size_t n)
{
    return true;
}

template <typename Tbuff, typename Tview>
std::pair<size_t, bool> MemStrBuff<Tbuff, Tview>::FindStr(const std::string& str)
{
    size_t line{};
    return { line, true };
}


#if 0
/////////////////////////////////////////////////////////////////////////////
int MStrBuff::ChangeStr(size_t str, const char* pStr, size_t len)
{
    size_t n = str;
    //TPRINT(("ChangeStr %d\n", n));
    int rc;

    if (n >= m_nStrCount)
        return 0;

    StrBuff* pSBuff = GetBuff(&n);
    if (!ASSERT(pSBuff != NULL))
    {
        return 0;
    }

    rc = pSBuff->ChangeStr(n, pStr, len);
    if (rc < 0)
    {
        rc = SplitBuff(pSBuff, n);
        if (!rc)
        {
            n = str;
            rc = -1;
            pSBuff = GetBuff(&n);
            if (pSBuff)
                rc = pSBuff->ChangeStr(n, pStr, len);
        }
        if (rc < 0)
            TPRINT(("Error %d\n", rc));
    }

    if (pSBuff)
        rc = pSBuff->ReleaseBuff();

    m_fChanged = 1;
    return 0;
}


int MStrBuff::AddStr(size_t str, const char* pStr, size_t len)
{
    int rc;
    if (str > m_nStrCount)
        return 0;

    size_t n = str;
    if (len == -1)
        len = strlen(pStr) + 1;

    //TPRINT(("AddStr n=%d l=%d\n", n, len));

    StrBuff* pSBuff = GetBuff(&n);
    if (!pSBuff)
    {
        //TPRINT(("Str not found\n"));
        pSBuff = m_pSBuff;
        size_t count = 0;
        while (pSBuff && pSBuff->m_next)
        {
            count += pSBuff->m_StrCount;
            pSBuff = pSBuff->m_next;
        }

        //    if(!ASSERT(n <= count + STR_NUM))
        //    {
        //      return -1;
        //    }

        if (!pSBuff)
        {
            StrBuff* pNewBuff = new StrBuff;
            if (!ASSERT(pNewBuff != NULL))
            {
                return -1;
            }

            if (pSBuff)
            {
                pSBuff->m_next = pNewBuff;
                pNewBuff->m_prev = pSBuff;
            }
            else
            {
                m_pSBuff = pNewBuff;
            }

            pSBuff = GetBuff(&n);
            if (!ASSERT(pSBuff != NULL))
                return -1;
        }
    }

    rc = pSBuff->AddStr(n, pStr, len);
    if (rc < 0)
    {
        if (n == pSBuff->m_StrCount)
        {
            //TPRINT(("Last line %d. Create new buff\n", n));
            StrBuff* pNewBuff = new StrBuff;
            if (!ASSERT(pNewBuff != NULL))
                return -1;

            pSBuff->m_next = pNewBuff;
            pNewBuff->m_prev = pSBuff;

            n = str;
            pSBuff = GetBuff(&n);
            if (!ASSERT(pSBuff != NULL))
                return -1;

            rc = pSBuff->AddStr(n, pStr, len);
        }
        else
        {
            rc = SplitBuff(pSBuff, n);
            if (!rc)
            {
                rc = -1;
                n = str;
                pSBuff = GetBuff(&n);
                if (pSBuff)
                    rc = pSBuff->AddStr(n, pStr, len);
            }
        }
        if (rc < 0)
            TPRINT(("Error %d\n", rc));
    }

    if (rc >= 0)
        ++m_nStrCount;

    if (pSBuff)
        pSBuff->ReleaseBuff();

    m_fChanged = 1;

    //TPRINT(("rc=%d count=%d\n", rc, m_nStrCount));
    return rc;
}


int MStrBuff::DelStr(size_t n)
{
    //TPRINT(("DelStr %d\n", n));

    StrBuff* pSBuff = GetBuff(&n);
    if (!pSBuff)
        return 0;
    int rc = pSBuff->DelStr(n);
    if (rc >= 0)
        --m_nStrCount;

    if (!pSBuff->m_StrCount)
    {
        //TPRINT(("EmptyBlock\n"));
        DelBuff(pSBuff);
    }
    else
        pSBuff->ReleaseBuff();

    m_fChanged = 1;

    return 0;
}


int MStrBuff::SplitBuff(StrBuff* pBuff, size_t nline)
{
    TPRINT(("SplitBuff n=%d\n", nline));
    size_t off = (!nline) ? 0 : pBuff->m_pStrOffset[nline - 1];
    size_t l = 0;
    if (pBuff->m_StrCount == STR_NUM)
    {
        //все строки уже заняты
        if (nline < STR_NUM / 3)
            // 1/3
            l = STR_NUM / 3;
        else if (nline < (STR_NUM / 3) * 2)
            // 1/2
            l = STR_NUM / 2;
        else
            // 2/3
            l = (STR_NUM / 3) * 2;

        if (pBuff->m_pStrOffset[l] > (BUFF_SIZE / 3) * 2)
            //здесь очень длинные строки
            // 2/3
            for (l = 0; pBuff->m_pStrOffset[l] < (BUFF_SIZE / 3) * 2; ++l);
    }
    else
    {
        if (off < BUFF_SIZE / 3)
            // 1/3
            for (l = 0; pBuff->m_pStrOffset[l] < BUFF_SIZE / 3; ++l);
        else if (off < (BUFF_SIZE / 3) * 2)
            // 1/2
            for (l = 0; pBuff->m_pStrOffset[l] < BUFF_SIZE / 2; ++l);
        else
            // 2/3
            for (l = 0; pBuff->m_pStrOffset[l] < (BUFF_SIZE / 3) * 2; ++l);
    }

    StrBuff* pNewBuff = new StrBuff;
    if (!ASSERT(pNewBuff != NULL))
        return -1;

    pNewBuff->m_next = pBuff->m_next;
    pNewBuff->m_prev = pBuff;
    pBuff->m_next = pNewBuff;
    if (pNewBuff->m_next)
        pNewBuff->m_next->m_prev = pNewBuff;

    char* pMBuff = pBuff->GetBuff();
    char* pnewBuffData = pNewBuff->GetBuff();
    if (!pMBuff || !pnewBuffData)
    {
        pBuff->ReleaseBuff();
        pNewBuff->ReleaseBuff();

        TPRINT(("ERROR GetBuff\n"));
        return -1;
    }

    pBuff->m_fMod = 1;
    pNewBuff->m_fMod = 1;

    size_t begin = pBuff->m_pStrOffset[l - 1];
    size_t end = pBuff->m_pStrOffset[pBuff->m_StrCount - 1];
    memcpy(pnewBuffData, pMBuff + begin, end - begin);

    for (size_t i = 0; i + l < pBuff->m_StrCount; ++i)
        pNewBuff->m_pStrOffset[i] = pBuff->m_pStrOffset[i + l] - begin;

    pNewBuff->m_StrCount = pBuff->m_StrCount - l;
    pBuff->m_StrCount = l;
    TPRINT(("n=%d n1=%d n2=%d\n", pBuff->m_StrCount, l, pNewBuff->m_StrCount));

    pBuff->ReleaseBuff();
    pNewBuff->ReleaseBuff();

    if (nline < l)
        TPRINT(("SplitBuff at n=%d same buff\n", l));
    else
        TPRINT(("SplitBuff at n=%d new buff\n", l));

    return 0;
}


int MStrBuff::DelBuff(StrBuff* pBuff)
{
    //TPRINT(("DelBuff\n"));
    if (pBuff == m_pCurBuff)
        m_pCurBuff = NULL;

    if (!pBuff->m_prev)
    {
        //del first
        if (pBuff->m_next)
        {
            //not most last block
            m_pSBuff = pBuff->m_next;
            m_pSBuff->m_prev = NULL;
            delete pBuff;
        }
    }
    else
    {
        pBuff->m_prev->m_next = pBuff->m_next;
        if (pBuff->m_next)
            pBuff->m_next->m_prev = pBuff->m_prev;
        delete pBuff;
    }
    return 0;
}


int MStrBuff::Find(const char* pStr)
{
    size_t len = strlen(pStr);

    for (size_t i = 0; i < m_nStrCount; ++i)
    {
        size_t l;
        char* p = GetStr(i, &l);
        if (p && l && l >= len)
        {
            int rc = strncmp(p, pStr, len);

            if (!rc)
                return (int)i;
        }
    }

    return -1;
}

#endif

template class BuffPool<std::string>;
template class SBuff<std::string, std::string_view>;
template class StrBuff<std::string, std::string_view>;
template class MemStrBuff<std::string, std::string_view>;
