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
#include "ConsoleInput.h"

#include <sstream>


std::string ConsoleInput::CastKeyCode(input_t code)
{
    std::string modificator;
    std::string mouseKey;
    std::string keyType;
    std::string msg;

    if((code & K_MODMASK) && !(code & K_USER))
        switch(code & K_MODMASK)
        {
        case K_SHIFT:
            modificator = "S+"; 
            break;
        case K_CTRL:
            modificator = "C+"; 
            break;
        case K_ALT:
            modificator = "A+"; 
            break;
        case (K_SHIFT | K_CTRL):
            modificator = "S+C+"; 
            break;
        case (K_SHIFT | K_ALT):
            modificator = "S+A+"; 
            break;
        case (K_CTRL | K_ALT):
            modificator = "C+A+"; 
            break;
        case (K_SHIFT | K_CTRL | K_ALT):
            modificator = "S+C+A+"; 
            break;
        
        //mouse
        case K_MOUSE2:
            mouseKey = "2"; 
            break;
        case K_MOUSE3:
            mouseKey = "3"; 
            break;
        case K_MOUSEW:
            mouseKey = "W"; 
            break;
        }

    if(code & K_TYPEMASK)
        switch(code & K_TYPEMASK)
        {
        case K_UP:
            keyType = "Up"; 
            break;
        case K_DOWN:
            keyType = "Down"; 
            break;
        case K_RIGHT:
            keyType = "Right"; 
            break;
        case K_LEFT:
            keyType = "Left"; 
            break;
        case K_INSERT:
            keyType = "Insert"; 
            break;
        case K_DELETE:
            keyType = "Delete"; 
            break;
        case K_HOME:
            keyType = "Home"; 
            break;
        case K_END:
            keyType = "End"; 
            break;
        case K_PAGEUP:
            keyType = "PgUp"; 
            break;
        case K_PAGEDN:
            keyType = "PgDn"; 
            break;
        case K_F1:
            keyType = "F1"; 
            break;
        case K_F2:
            keyType = "F2"; 
            break;
        case K_F3:
            keyType = "F3"; 
            break;
        case K_F4:
            keyType = "F4"; 
            break;
        case K_F5:
            keyType = "F5"; 
            break;
        case K_F6:
            keyType = "F6"; 
            break;
        case K_F7:
            keyType = "F7"; 
            break;
        case K_F8:
            keyType = "F8"; 
            break;
        case K_F9:
            keyType = "F9"; 
            break;
        case K_F10:
            keyType = "F10"; 
            break;
        case K_F11:
            keyType = "F11"; 
            break;
        case K_F12:
            keyType = "F12"; 
            break;
        case K_PRESS:
            keyType = "PRESS"; 
            break;
        case K_RELEASE:
            keyType = "RELEASE"; 
            break;

        //mouse
        case K_MOUSE:
            keyType = "Mouse";
            msg = "_   m=" + mouseKey + " x=" + std::to_string(K_GET_X(code)) + " y=" + std::to_string(K_GET_Y(code));
            break;
        case K_MOUSEKL:
            keyType = "Mouse";
            msg = "L   m=" + mouseKey + " x=" + std::to_string(K_GET_X(code)) + " y=" + std::to_string(K_GET_Y(code));
            break;
        case K_MOUSEKR:
            keyType = "Mouse";
            msg = "R   m=" + mouseKey + " x=" + std::to_string(K_GET_X(code)) + " y=" + std::to_string(K_GET_Y(code));
            break;
        case K_MOUSEKM:
            keyType = "Mouse";
            msg = "M   m=" + mouseKey + " x=" + std::to_string(K_GET_X(code)) + " y=" + std::to_string(K_GET_Y(code));
            break;
        case K_MOUSEKUP:
            keyType = "Mouse";
            msg = "KUP m=" + mouseKey + " x=" + std::to_string(K_GET_X(code)) + " y=" + std::to_string(K_GET_Y(code));
            break;
        case K_MOUSEWUP:
            keyType = "Mouse";
            msg = "WUP m=" + mouseKey + " x=" + std::to_string(K_GET_X(code)) + " y=" + std::to_string(K_GET_Y(code));
            break;
        case K_MOUSEWDN:
            keyType = "Mouse";
            msg = "WDN m=" + mouseKey + " x=" + std::to_string(K_GET_X(code)) + " y=" + std::to_string(K_GET_Y(code));
            break;

        case K_RESIZE:
            keyType = "Resize";
            msg = "x=" + std::to_string(K_GET_X(code)) + " y=" + std::to_string(K_GET_Y(code));
            break;
        case K_FOCUSSET:
            keyType = "SetFocus";
            break;
        case K_FOCUSLOST:
            keyType = "LostFocus";
            break;
        case K_TIME:
            keyType = "Time";
            break;
        case K_EXIT:
            keyType = "Exit";
            break;
        case K_APP:
            keyType = "App";
            break;

        default:
            if(code & K_USER)
                keyType = "User";
            else
                keyType = "???";
            break;
        }

    if((code & K_TYPEMASK) == K_SYMBOL)
    {
        if(K_GET_CODE(code) <= K_SPACE)
        {
            switch(K_GET_CODE(code))
            {
            case K_TAB:
                msg = "Tab"; 
                break;
            case K_ENTER:
                msg = "Enter"; 
                break;
            case K_ESC:
                msg = "Esc"; 
                break;
            case K_SPACE:
                msg = "Space"; 
                break;
            default:
                msg = "???"; 
                break;
            }
        }
        else if(K_GET_CODE(code) < K_BS)
            //ascii
            msg = char(K_GET_CODE(code));
        else if (K_GET_CODE(code) == K_BS)
            msg = "BS";
        else
        {
            //unicode
            auto tohex = [](uint32_t val) {
                std::stringstream sstream;
                sstream << std::hex << val;
                return sstream.str();
            };
            msg = "0x" + tohex(K_GET_CODE(code));
        }
    }

    std::stringstream out;
    out << "key=" << std::hex << code << " '" << modificator << keyType << " " << msg << "'";
    
    return out.str();
}

