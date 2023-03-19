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

#include "Diff.h"
#include "utils/logger.h"

namespace _Editor
{

void HashBuff::ChangedLines(size_t first, size_t last, bool calcHash)
{
    m_firstChanged = first;
    m_lastChanged = last;
    auto n = last - first + 1;

    m_changed.resize(n);
    m_hash.resize(n);
    if (calcHash)
    {
        for (size_t i = 0; i < n; ++i)
        {
            auto str = m_editor->GetStr(i);
            m_hash[i] = std::hash<decltype(str)>{}(str);
        }
    }
    else
    {
        for (size_t i = 0; i < n; ++i)
            m_changed[i] = true;
    }
}

Diff::Diff(EditorPtr editor1, EditorPtr editor2, bool withoutSpace,
    size_t first1, size_t last1, size_t first2, size_t last2)
    : m_diffBuff{{editor1, withoutSpace, first1, last1}, {editor1, withoutSpace, first2, last2}}
{
    auto f1 = first1;
    auto l1 = last1;
    auto f2 = first2;
    auto l2 = last2;

    LOG(DEBUG) << __FUNC__ << "f1=" << f1 << " l1=" << l1 << " f2=" << f2 << " l2=" << l2;

    //skip the identical lines from beginning
    for (; f1 <= l1 && f2 <= l2; ++f1, ++f2)
    {
        auto str1 = editor1->GetStr(f1);
        auto str2 = editor2->GetStr(f2);

        if (str1 != str2)
            break;
    }
    if (f1 > l1 && f2 > l2)
    {
        LOG(DEBUG) << "diff same";
        return;
    }

    //skip the identical lines from end
    for (; l1 > f1 && l2 > f2; --l1, --l2)
    {
        auto str1 = editor1->GetStr(l1);
        auto str2 = editor2->GetStr(l2);

        if (str1 != str2)
            break;
    }

    if (f1 > l1)
    {
        LOG(DEBUG) << "diff f2-l2 inserted";
        m_diffBuff[1].ChangedLines(f2, l2);
    }
    else if (f2 > l2)
    {
        LOG(DEBUG) << "diff f1-l1 deleted";
        m_diffBuff[0].ChangedLines(f1, l1);
    }
    else
    {
        LOG(DEBUG) << "diff f1-l1 f2-l2";
        m_diffBuff[0].ChangedLines(f1, l1, true);
        m_diffBuff[1].ChangedLines(f2, l2, true);
    }
}

bool Diff::Compare()
{
    if (m_diffBuff[0].m_hash.empty() && m_diffBuff[1].m_hash.empty())
        return false;

    //MaxD == Maximum possible deviation from centre diagonal vector
    //which can't be more than the largest Array (with upperlimit = MAX_DIAGONAL)
    m_maxDiagonal = std::min(std::max(m_diffBuff[0].m_hash.size(), m_diffBuff[1].m_hash.size()), MAX_DIAGONAL);

    //allocate the memory for diagonals
    m_diagF.resize(m_maxDiagonal * 2 + 1);
    m_diagB.resize(m_maxDiagonal * 2 + 1);

    int64_t bottom1 = 0;
    int64_t bottom2 = 0;
    int64_t top1 = m_diffBuff[0].m_hash.size() - 1;
    int64_t top2 = m_diffBuff[1].m_hash.size() - 1;

    RecursiveDiff(bottom1 - 1, bottom2 - 1, top1, top2);

    MergeEmptyLine();

    return true;
}

void Diff::RecursiveDiff(int64_t bottom1, int64_t bottom2, int64_t top1, int64_t top2)
{
    //check if just all additions or all deletions
    if (top1 == bottom1)
    {
        AddChanges(bottom1, bottom2, top1, top2, ChangeType::AddRange);
        return;
    }
    else if (top2 == bottom2)
    {
        AddChanges(bottom1, bottom2, top1, top2, ChangeType::DelRange);
        return;
    }

    //Delta = offset of bottomright from topleft corner
    int64_t Delta = (top1 - bottom1) - (top2 - bottom2);

    //initialize the forward and backward diagonal vectors including the outer bounds ...
    m_diagF[m_maxDiagonal + 0] = bottom1;
    m_diagB[m_maxDiagonal + Delta] = top1;

    m_diagF[m_maxDiagonal + top1 - bottom1] = top1;
    m_diagB[m_maxDiagonal + top1 - bottom1] = top1;

    //the following avoids the -(top2 - bottom2) vectors being assigned invalid values.
    if (Delta > 0)
    {
        m_diagF[m_maxDiagonal - (top2 - bottom2 + 1)] = bottom1;
        m_diagB[m_maxDiagonal - (top2 - bottom2 + 1)] = bottom1;
    }

    //When the forward and backward arrays cross over at some point the
    //curr1 and curr2 values represent a relative mid-point on the
    //'shortest common sub-sequence' path
    //By recursively finding these points the whole path can be constructed

    //MAKE INCREASING OSCILLATIONS ABOUT CENTRE DIAGONAL UNTIL A FORWARD
    //DIAGONAL VECTOR IS GREATER THAN OR EQUAL TO A BACKWARD DIAGONAL.
    int64_t curr1, curr2;
    for (int64_t D = 1; D <= m_maxDiagonal; ++D)
    {
        //forward loop
        //k == index of current diagonal vector and
        //will oscillate (in increasing swings) between -MaxD and MaxD
        int64_t k = -D;

        //stop going outside the grid
        while (k < -(top2 - bottom2))
            k += 2;

        int64_t i = std::min(D, (top1 - bottom1) - 1);
        while (k <= i)
        {
            //derive curr1 from the largest of adjacent vectors
            if (k == -D || (k < D && m_diagF[m_maxDiagonal + k - 1] < m_diagF[m_maxDiagonal + k + 1]))
                curr1 = m_diagF[m_maxDiagonal + k + 1];
            else
                curr1 = m_diagF[m_maxDiagonal + k - 1] + 1;

            //derive curr2 (see above)
            curr2 = curr1 - bottom1 + bottom2 - k;

            while (curr1 < top1 && curr2 < top2 &&
                m_diffBuff[0].m_hash[curr1 + 1] == m_diffBuff[1].m_hash[curr2 + 1])
            {
                ++curr1;
                ++curr2;
            }

            //update current vector
            m_diagF[m_maxDiagonal + k] = curr1;

            //check if a vector in diagVecF overlaps the corresp. diagVecB vector.
            //If a crossover point found here then further recursion is required.
            if ((Delta & 1) && k > -D + Delta && k < D + Delta &&
                m_diagF[m_maxDiagonal + k] >= m_diagB[m_maxDiagonal + k])
            {
                //find subsequent points by recursion
                //Delta & k are simply reused to store the curr1 & curr2 values
                Delta = curr1;
                k = curr2;

                //ignore trailing matches in lower block
                while (curr1 > bottom1 && curr2 > bottom2 &&
                    m_diffBuff[0].m_hash[curr1] == m_diffBuff[1].m_hash[curr2])
                {
                    --curr1;
                    --curr2;
                }

                RecursiveDiff(bottom1, bottom2, curr1, curr2);

                //and with the upper block (Delta & k are stored curr1 & curr2)
                RecursiveDiff(Delta, k, top1, top2);

                return;
            }
            k += 2;
        }

        //backward loop
        //k will oscillate (in increasing swings) between -MaxD and MaxD
        k = -D + Delta;

        //stop going outside grid and also ensure we remain within the
        //diagVecB[] and diagVecF[] array bounds.
        //in the backward loop it is necessary to test the bottom left corner
        while (k < -(top2 - bottom2))
            k += 2;

        i = std::min(D + Delta, (top1 - bottom1) - 1);
        while (k <= i)
        {
            //derive curr1 from the adjacent vectors
            if (k == D + Delta || (k > -D + Delta && m_diagB[m_maxDiagonal + k + 1] > m_diagB[m_maxDiagonal + k - 1]))
                curr1 = m_diagB[m_maxDiagonal + k - 1];
            else
                curr1 = m_diagB[m_maxDiagonal + k + 1] - 1;

            curr2 = curr1 - bottom1 + bottom2 - k;

            while (curr1 > bottom1 && curr2 > bottom2 &&
                m_diffBuff[0].m_hash[curr1] == m_diffBuff[1].m_hash[curr2])
            {
                --curr1;
                --curr2;
            }

            //update current vector
            m_diagB[m_maxDiagonal + k] = curr1;

            //check if a crossover point reached
            if (!(Delta & 1) && k >= -D && k <= D && m_diagF[m_maxDiagonal + k] >= m_diagB[m_maxDiagonal + k])
            {
                if ((bottom1 + 1 == top1) && (bottom2 + 1 == top2))
                {
                    //changed string
                    AddChanges(bottom1, bottom2, top1, top2, ChangeType::AddDel);
                }
                else
                {
                    //otherwise process the lower block
                    RecursiveDiff(bottom1, bottom2, curr1, curr2);

                    //strip leading matches in upper block
                    while (curr1 < top1 && curr2 < top2 &&
                        m_diffBuff[0].m_hash[curr1 + 1] == m_diffBuff[1].m_hash[curr2 + 1])
                    {
                        ++curr1;
                        ++curr2;
                    }

                    //and process the upper block
                    RecursiveDiff(curr1, curr2, top1, top2);
                }

                return;
            }
            k += 2;
        }
    }
}

void Diff::AddChanges(int64_t bottom1, int64_t bottom2, int64_t top1, int64_t top2, ChangeType type)
{
    switch (type)
    {
    case ChangeType::DelRange:
    {
        for (auto i = bottom1 + 1; i <= top1; ++i)
            m_diffBuff[0].m_changed[i] = true;
        break;
    }

    case ChangeType::AddRange:
    {
        for (auto i = bottom2 + 1; i <= top2; ++i)
            m_diffBuff[1].m_changed[i] = true;
        break;
    }

    case ChangeType::AddDel:
    {
        m_diffBuff[0].m_changed[bottom1 + 1] = true;
        m_diffBuff[1].m_changed[bottom2 + 1] = true;
        break;
    }
    }
}

void Diff::MergeEmptyLine()
{
    //???
}

} //namespace _Editor
