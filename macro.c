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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include "system.h"
#include "macro.h"

#define DEFINEINVOKER(n,l)			\
  static Cell invoke_##n (Function f)		\
  {						\
    Function_##n f##n;				\
    f##n = (Function_##n)f;			\
    return (*f##n) l;				\
  }

typedef Cell (*Function_0) (void);
typedef Cell (*Function_1) (Cell);
typedef Cell (*Function_2) (Cell, Cell);
typedef Cell (*Function_3) (Cell, Cell, Cell);
typedef Cell (*Function_4) (Cell, Cell, Cell, Cell);

static Macro *macros = NULL;

DEFINEINVOKER (0, ())
DEFINEINVOKER (1, (sys.task.stack[0]))
DEFINEINVOKER (2, (sys.task.stack[1], sys.task.stack[0]))
DEFINEINVOKER (3, (sys.task.stack[2], sys.task.stack[1],
		   sys.task.stack[0]))
DEFINEINVOKER (4, (sys.task.stack[3], sys.task.stack[2],
		   sys.task.stack[1], sys.task.stack[0]))

static int
truncate_name (char *c_string)
{
  int length;

  length = strlen (c_string);
  return (length > MACRO_NAME_SIZE) ? MACRO_NAME_SIZE : length;
}

static int
macro_has_name (Character *name, char *c_string)
{
  int i, length;

  length = truncate_name (c_string);
  if (length != *name++)
    return 0;
  for (i = 0; i < length; i++)
    if (toupper (c_string[i]) != toupper (name[i]))
      return 0;
  return 1;
}

void
register_macro (char *name, Function invokee,
                int arguments_number)
{
  static Invoker_Function invokers[] =
    {
     invoke_0, invoke_1, invoke_2, invoke_3, invoke_4
    };
  Macro *object;

  object = (Macro *) malloc (sizeof(Macro));
  if (object == NULL || arguments_number > 4)
    {
      fatal_error ("REGISTER MACRO FAILED");
    }
  else
    {
      int length;

      length = truncate_name (name);
      memcpy ((char *) object->name, (char *) name,
              sizeof (char) * length);
      object->name[length] = 0;
      object->invokee = invokee;
      object->arguments_number = arguments_number;
      object->invoker = invokers[arguments_number];
      object->link = macros;
      macros = object;
    }
}

Macro *
find_macro (Character *name)
{
  Macro *object;

  object = macros;
  while (object != NULL)
    {
      if (macro_has_name (name, object->name))
	return object;
      object = object->link;
    }
  return NULL;
}

void
clear_macros (void)
{
  Macro *temp;

  while (macros != NULL)
    {
      temp = macros;
      macros = temp->link;
      free ((char *) temp);
    }
}
