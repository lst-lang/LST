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

#define MACRO_NAME_SIZE 32

typedef Cell (*Invoker_Function) (Function);

struct _Macro
{
  char name[MACRO_NAME_SIZE];
  Function invokee;
  Cell arguments_number;
  Invoker_Function invoker;
  struct _Macro *link;
};
typedef struct _Macro Macro;

void register_macro (char *, Function, int);
Macro *find_macro (Character *);
void clear_macros (void);
