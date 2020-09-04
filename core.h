/******************************************************************************
*   This file is part of LST                                                  *
*                                                                             *
*   LST is free software; you can redistribute it and/or modify               *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 3 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with LST.  If not, see https://www.gnu.org/licenses.                *
******************************************************************************/

Cell dump (Cell, Cell);
Cell macro_emit_instruction_slot (Cell);
Cell macro_emit_instruction_word (Cell);
Cell macro_fill_instruction_word (Cell);
Cell macro_tail_recurse (void);
Cell macro_recurse (void);
Cell macro_define (Cell, Cell);
Cell macro_end_define (Cell);
Cell macro_find_word (Cell, Cell);
Cell macro_accept (Cell, Cell);
Cell macro_skip_delimiters (Cell);
Cell macro_find_delimiter (Cell);
Cell macro_word (Cell);
Cell macro_number (void);
Cell macro_immediate (void);
Cell macro_literal (Cell);
Cell macro_emit (Cell);
Cell macro_move (Cell, Cell, Cell);
Cell macro_cmove (Cell, Cell, Cell);
Cell macro_cmove_up (Cell, Cell, Cell);
Cell macro_compare (Cell, Cell, Cell, Cell);
Cell macro_compare_igcase (Cell, Cell, Cell, Cell);
Cell macro_display_syserr (Cell);
Cell macro_align_number (Cell, Cell);
Cell macro_key (void);
void register_core_macros (void);
