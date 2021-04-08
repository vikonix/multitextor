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
#include "Dialogs/EditorDialogs.h"
#include "Version.h"


std::list<control> dlgAbout
{
    {CTRL_TITLE,                        "About " EDITOR_NAME,                                   0, NULL,  1,  0, 45, 11},

    {CTRL_STATIC,                       EDITOR_NAME_C " Version " EDITOR_VERSION,               0, NULL,  1,  1, 51},
    {CTRL_STATIC,                       "Cross-platform text editor",                           0, NULL,  1,  2, 41},
    {CTRL_STATIC,                       "https://github.com/vikonix/multitextor",               0, NULL,  1,  4, 41},
    {CTRL_STATIC,                       "Copyright (C) " COPYRIGHT_YEAR " " COPYRIGTH_OWNER,    0, NULL,  1,  5, 41},
    {CTRL_LINE,                         "",                                                     0, NULL,  1,  7, 41},

    {CTRL_DEFBUTTON | CTRL_ALIGN_LEFT,  "Ok",                                               ID_OK, NULL, 29, 8}
};

AboutDialog::AboutDialog(pos_t x, pos_t y)
    : Dialog(dlgAbout, x, y)
{}

/////////////////////////////////////////////////////////////////////////////
std::list<control> dlgExit
{
    {CTRL_TITLE,                    "Exit",                             0, NULL,  1, 0, 35, 7},

    {CTRL_STATIC,                   "Do you want to exit program ?",    0, NULL,  1, 1, 31},
    {CTRL_LINE,                     "",                                 0, NULL,  1, 3, 31},

    {CTRL_BUTTON | CTRL_ALIGN_LEFT, "Yes",                          ID_OK, NULL, 14, 4,  0,  0, "Exit from application"},
    {CTRL_BUTTON | CTRL_ALIGN_LEFT, "Cancel",                   ID_CANCEL, NULL, 22, 4}
};

ExitDialog::ExitDialog(pos_t x, pos_t y)
    : Dialog(dlgExit, x, y)
{}
