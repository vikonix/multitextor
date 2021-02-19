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
#include "Utils/SymbolType.h"

#include <cwctype>


symbol_t GetSymbolType(char16_t wc)
{
    const int LexicalTab[] =
    {
    //  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    //                                     TAB LF          CR
        0,  4,  4,  4,  4,  4,  4,  4,  4,  1,  0,  4,  4,  0,  4,  4,
    //                                         EOF
        4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  4,  4,  4,  4,  4,
    // ' '  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
        1,  5,  2,  5,  5,  5,  5,  2,  5,  5,  5,  5,  5,  5,  5,  5,
    //  0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
        6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  5,  5,  5,  5,  5,
    //  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
        5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    //  P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
        6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  3,  5,  5,  6,
    //  `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
        2,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    //  p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~  ''
        6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  5,  5,  5,  4
    };

    if (wc < 0x80)
        return static_cast<symbol_t>(LexicalTab[wc]);
    else
    {
        if (std::iswalnum(wc))
            return symbol_t::alnum;
        else
            return symbol_t::other;
    }
}
