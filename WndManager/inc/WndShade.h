/*
FreeBSD License

Copyright (c) 2020 vikonix: valeriy.kovalev.software@gmail.com
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

#include "Types.h"
#include <vector>

#define SHADE_PAINT  0x20
#define SHADE_SAVE   0x10

#define SHADE_TOP    0x01
#define SHADE_LEFT   0x02
#define SHADE_RIGHT  0x04
#define SHADE_BOTTOM 0x08
#define SHADE_ALL    0x0f


class Shade final
{
    pos_t   m_x;
    pos_t   m_y;
    pos_t   m_sizex;
    pos_t   m_sizey;
    int     m_mode;

    std::vector<cell_t> m_pSaveT;
    std::vector<cell_t> m_pSaveL;
    std::vector<cell_t> m_pSaveR;
    std::vector<cell_t> m_pSaveB;

    std::vector<cell_t> m_SaveTL;
    std::vector<cell_t> m_SaveTR;
    std::vector<cell_t> m_SaveBL;
    std::vector<cell_t> m_SaveBR;

public:
    Shade() = delete;
    explicit Shade(pos_t x, pos_t y, pos_t sizex, pos_t sizey, int mode = SHADE_ALL);
    ~Shade();

    bool Paint();
    bool Hide();
    bool Discard() {m_mode = 0; return 0;}
};

