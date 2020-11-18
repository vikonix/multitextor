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


#define K_UNUSED    0x00000000
#define K_ERROR     0xffffffff

#define K_SYMBOL    0x00000000
#define K_TAB       0x00000009
#define K_ENTER     0x0000000d
#define K_ESC       0x0000001b
#define K_SPACE     0x00000020
#define K_BS        0x0000007f

#define K_UP        0x01000000
#define K_DOWN      0x02000000
#define K_RIGHT     0x03000000
#define K_LEFT      0x04000000
#define K_INSERT    0x05000000
#define K_DELETE    0x06000000
#define K_HOME      0x07000000
#define K_END       0x08000000
#define K_PAGEUP    0x09000000
#define K_PAGEDN    0x0a000000
#define K_F1        0x10000000
#define K_F2        0x11000000
#define K_F3        0x12000000
#define K_F4        0x13000000
#define K_F5        0x14000000
#define K_F6        0x15000000
#define K_F7        0x16000000
#define K_F8        0x17000000
#define K_F9        0x18000000
#define K_F10       0x19000000
#define K_F11       0x1a000000
#define K_F12       0x1b000000
#define K_PRESS     0x1c000000 //key press
#define K_RELEASE   0x1d000000 //key release

#define K_RESIZE    0x20000000 //window resized
#define K_FOCUSSET  0x21000000 //set focus
#define K_FOCUSLOST 0x22000000 //lost focus
#define K_TIME      0x23000000 //time puls
#define K_EXIT      0x24000000 //exit command
#define K_CLOSE     0x25000000 //close window
#define K_SELECT    0x26000000 //select some item
#define K_MENU      0x27000000 //call menu
#define K_CONTROL   0x28000000 //control element id
#define K_REFRESH   0x29000000 //refresh screen
#define K_APP       0x2a000000 //application command

#define K_MOUSE     0x40000000 //mouse moved
#define K_MOUSEKL   0x41000000 //left button
#define K_MOUSEKM   0x42000000 //middle buton
#define K_MOUSEKR   0x43000000 //right button
#define K_MOUSEKUP  0x44000000 //button released
#define K_MOUSEWUP  0x45000000 //wheel up
#define K_MOUSEWDN  0x46000000 //wheel down

#define K_USER      0x80000000 //creating user defined codes

#define K_SHIFT     0x00010000
#define K_CTRL      0x00020000
#define K_ALT       0x00040000

#define K_MOUSE2    0x00200000 //double click
#define K_MOUSE3    0x00400000 //triple click
#define K_MOUSEW    0x00800000 //wheeled


#define K_TYPEMASK  0xff000000
#define K_MODMASK   0x00ff0000
#define K_CODEMASK  0x0000ffff

#define K_X_MASK    0x0000ff00 //mouse x coord/ window x size
#define K_Y_MASK    0x000000ff //mouse y coord/ window y size

#define K_MAKE_COORD_CODE(k, x, y) ((k) | ((x) << 8) | (y))
#define K_GET_X(k)      (((k) & K_X_MASK) >> 8)
#define K_GET_Y(k)      ((k) & K_Y_MASK)
#define K_GET_CODE(k)   ((k) & K_CODEMASK)

/*
                 Linux            PuTTY             XTrem
             shift ctrl alt   shift ctrl alt    shift ctrl alt
 F1-F10        X                +                 +

 TAB                     +      +
 Enter                   +                +       +         +
 BS                  +   X      +         +                 +

 Insert                                   +                 +
 Delete                                   +       +         +
 Home                                     +       +         +
 End                                      +       +         +
 PgUp                                     +       +         +
 PgDn                                     +       +         +

 Up                                  +    +                 +
 Down                                +    +                 +
 Left                                +    +                 +
 Right                               +    +                 +

 a-z }\        X    X    X      +    +    +       +    +    +
 Space              X    X           +    +            +    +
*/

