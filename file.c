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
#include "execute.h"
#include "file.h"

static char *
malloc_not_fail (Cell u)
{
  char *p;

  p = malloc (sizeof (char) * u);
  if (p == NULL)
    THROW (-59);
  return p;
}

static char *
make_c_string (Cell c_addr, Cell u)
{
  Cell i;
  Character *string;
  char *name;

  string = (Character *) A (c_addr);
  name = malloc_not_fail (sizeof (char) * (u + 1));
  for (i = 0; i < u; i++)
    name[i] = string[i];
  name[i] = 0;
  return name;
}

Cell
macro_fptrs (Cell u)
{
  return u * sizeof (FILE *);
}

Cell
macro_fopen (Cell c_addr, Cell u, Cell fam, Cell fp)
{
  static char *fam_to_mode[6] =
    { "r", "r+", "w", "rb", "rb+", "wb" };
  char *name;
  FILE *f;

  name = make_c_string (c_addr, u);
  f = fopen (name, fam_to_mode[fam]);
  free (name);
  if (f != NULL)
    {
      FILE **p;

      p = (FILE **) A (fp);
      *p = f;
      return 0;
    }
  else
    {
      return -1;
    }
}

Cell
macro_fclose (Cell fp)
{
  FILE *f;

  f = *(FILE **) A (fp);
  return fclose (f);
}

Cell
macro_fremove (Cell c_addr, Cell u)
{
  int result;
  char *name;

  name = make_c_string (c_addr, u);
  result = remove (name);
  free (name);
  return result;
}

Cell
macro_fread (Cell c_addr, Cell u, Cell fp)
{
  size_t result;
  Character *string;
  FILE *f;

  f = *(FILE **) A (fp);
  string = (Character *) A (c_addr);
  result = fread ((char *) string, sizeof (Character), u, f);
  return result / sizeof (Character);
}

Cell
macro_fwrite (Cell c_addr, Cell u, Cell fp)
{
  size_t result;
  Character *string;
  FILE *f;

  f = *(FILE **) A (fp);
  string = (Character *) A (c_addr);
  result = fwrite ((char *) string, sizeof (Character), u, f);
  return result / sizeof (Character);
}

Cell
macro_feof (Cell fp)
{
  return feof(*(FILE **) A (fp));
}

Cell
macro_ferror (Cell fp)
{
  return ferror(*(FILE **) A (fp));
}

Cell
macro_fseek (Cell offset, Cell fp, Cell from)
{
  return fseek (*(FILE **) A (fp),
		offset * sizeof (Character), from);
}

Cell
macro_ftell (Cell fp)
{
  return ftell (*(FILE **) A (fp)) / sizeof (Character);
}

void
register_file_macros (void)
{
  register_macro ("FPTRS", (Function) macro_fptrs, 1);
  register_macro ("FOPEN", (Function) macro_fopen, 4);
  register_macro ("FCLOSE", (Function) macro_fclose, 1);
  register_macro ("FREMOVE", (Function) macro_fremove, 2);
  register_macro ("FREAD", (Function) macro_fread, 3);
  register_macro ("FWRITE", (Function) macro_fwrite, 3);
  register_macro ("FEOF", (Function) macro_feof, 1);
  register_macro ("FERROR", (Function) macro_ferror, 1);
  register_macro ("FSEEK", (Function) macro_fseek, 3);
  register_macro ("FTELL", (Function) macro_ftell, 1);
}
