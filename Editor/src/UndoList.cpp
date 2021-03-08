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
#include "UndoList.h"
#include "Editor.h"

bool UndoList::Clear()
{
    m_rem.clear();
    m_editList.clear();
    m_undoList.clear();
    m_editIt = m_editList.end();
    m_undoIt = m_undoList.end();
    return true;
}

bool UndoList::AddEditCmd(cmd_t command, size_t line, size_t pos, size_t count, size_t len, const std::u16string& str)
{
    if (m_editIt != m_editList.end())
    {
        m_editList.erase(m_editIt, m_editList.end());
        m_editIt = m_editList.end();
    }
    size_t strlen = Editor::UStrLen(str);
    if (strlen > len)
        strlen = len;
    m_editList.push_back(EditCmd{ command, line, pos, len, count, std::move(str.substr(0, strlen)), m_rem });
    return true;
}

bool UndoList::AddUndoCmd(cmd_t command, size_t line, size_t pos, size_t count, size_t len, const std::u16string& str)
{
    if (m_undoIt != m_undoList.end())
    {
        m_undoList.erase(m_undoIt, m_undoList.end());
        m_undoIt = m_undoList.end();
    }
    size_t strlen = Editor::UStrLen(str);
    if (strlen > len)
        strlen = len;
    m_undoList.emplace_back(EditCmd{ command, line, pos, len, count, std::move(str.substr(0, strlen)), m_rem });
    return true;
}

std::optional<EditCmd> UndoList::GetEditCmd()
{
    if (m_editIt == m_editList.end())
        return std::nullopt;
    auto it = m_editIt;
    ++m_editIt;
    ++m_undoIt;
    return *it;
}

std::optional<EditCmd> UndoList::PeekEditCmd()
{
    if (m_editIt == m_editList.end())
        return std::nullopt;
    return *m_editIt;
}

std::optional<EditCmd> UndoList::GetUndoCmd()
{
    if (m_undoIt == m_undoList.begin())
        return std::nullopt;
    --m_editIt;
    --m_undoIt;
    return *m_undoIt;
}

std::optional<EditCmd> UndoList::PeekUndoCmd()
{
    if (m_undoIt == m_undoList.begin())
        return std::nullopt;
    auto it = m_undoIt;
    --it;
    return *it;
}
