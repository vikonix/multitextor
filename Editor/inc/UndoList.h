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

#include <string>
#include <list>
#include <optional>


enum class cmd_t
{
    CMD_NULL,
    CMD_END,             //end of command sequence
    CMD_BEGIN,           //begin of command sequence
    CMD_SET_POS,

    CMD_ADD_LINE,
    CMD_DEL_LINE,
    CMD_MERGE_LINE,
    CMD_SPLIT_LINE,
    CMD_ADD_SUBSTR,
    CMD_CHANGE_SUBSTR,
    CMD_CLEAR_SUBSTR,
    CMD_DEL_SUBSTR,
    CMD_REPLACE_SUBSTR,
    CMD_INDENT,
    CMD_UNDENT,
    CMD_CORRECT_TAB,
    CMD_SAVE_TAB,
    CMD_RESTORE_TAB,
    CMD_MARK
};

#define MAX_UNDO_SIZE 100000


struct EditCmd
{
    cmd_t             command{};
    size_t            line{};
    size_t            pos{};
    size_t            len{};
    size_t            count{};

    std::u16string    str{};
    std::string       remark{};
};


class UndoList
{
    std::string m_rem;

    std::list<EditCmd> m_editList;
    std::list<EditCmd> m_undoList;
    std::list<EditCmd>::iterator m_editIt;
    std::list<EditCmd>::iterator m_undoIt;

public:
  UndoList()  {}
  ~UndoList() {}

  void SetRemark(const std::string& rem) { m_rem = rem; }

  bool Clear();
  bool AddEditCmd(cmd_t command, size_t line, size_t pos, size_t count, size_t len, const std::u16string& str);
  bool AddUndoCmd(cmd_t command, size_t line, size_t pos, size_t count, size_t len, const std::u16string& str);

  std::optional<EditCmd> GetEditCmd();
  std::optional<EditCmd> PeekEditCmd();
  std::optional<EditCmd> GetUndoCmd();
  std::optional<EditCmd> PeekUndoCmd();
};
