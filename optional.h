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

Cell macro_fptrs (Cell);
Cell macro_fopen (Cell, Cell, Cell, Cell);
Cell macro_fclose (Cell);
Cell macro_fremove (Cell, Cell);
Cell macro_fread (Cell, Cell, Cell);
Cell macro_fwrite (Cell, Cell, Cell);
Cell macro_feof (Cell);
Cell macro_ferror (Cell);
Cell macro_fseek (Cell, Cell, Cell);
Cell macro_ftell (Cell);
Cell macro_f_store (Cell, Cell);
Cell macro_f_star (Cell, Cell);
Cell macro_f_plus (Cell, Cell);
Cell macro_f_minus (Cell, Cell);
Cell macro_f_slash (Cell, Cell);
Cell macro_f_less_than (Cell, Cell);
Cell macro_floats (Cell);
Cell macro_floor (Cell);
Cell macro_fnegate (Cell);
Cell macro_frot (Cell);
Cell macro_fround (Cell);
Cell macro_swap (Cell);
Cell macro_f_zero_less_than (Cell);
Cell macro_f_zero_equals (Cell);
Cell macro_ud_to_f (Cell, Cell, Cell);
Cell macro_dump (Cell, Cell);
Cell macro_see (void);
Cell macro_words (void);
Cell macro_int_dot (Cell);
Cell macro_long_dot (Cell, Cell);
Cell macro_float_dot (Cell);
void register_file_macros (void);
