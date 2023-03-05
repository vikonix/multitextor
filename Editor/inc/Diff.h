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

#include "EditorWnd.h"

namespace _Editor
{

constexpr size_t MAX_DIAGONAL{ 0xffffff }; //16meg

enum class ChangeType 
{
  AddRange = 1,
  DelRange,
  AddDel,
};

class HashBuff
{
    friend class Diff;

    EditorPtr           m_editor;
    bool                m_withoutSpace;

    size_t              m_first;            //diff interval
    size_t              m_last;
    size_t              m_firstChanged{};   //changed diff interval
    size_t              m_lastChanged{};

    std::vector<bool>   m_changed;//???
    std::vector<size_t> m_hash;

public:
    HashBuff(EditorPtr editor, bool withoutSpace, size_t first, size_t last)
        : m_editor{ editor }
        , m_withoutSpace{ withoutSpace }
        , m_first{first}
        , m_last{last}
    {}

    void ChangedLines(size_t first, size_t last, bool calcHash = false);
};

class Diff
{
protected:
    HashBuff    m_diffBuff[2];
    int64_t     m_maxDiagonal;
    std::vector<int64_t> m_diagF;
    std::vector<int64_t> m_diagB;


public:
    explicit Diff(EditorPtr editor1, EditorPtr editor2, bool withoutSpace,
        size_t first1, size_t last1,
        size_t first2, size_t last2
    );

    bool Compare();
    void RecursiveDiff(int64_t bottom1, int64_t bottom2, int64_t top1, int64_t top2);
    void AddChanges(int64_t bottom1, int64_t bottom2, int64_t top1, int64_t top2, ChangeType type);
    void MergeEmptyLine();
};

} //namespace _Editor
