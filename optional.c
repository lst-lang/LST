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
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include "execute.h"
#include "optional.h"

typedef float Floating;

static char *
allocate_c_string (Cell u)
{
  Cell _data_pointer;

  _data_pointer = *data_pointer;
  if ((Unsigned_Cell) (_data_pointer + u) > *vocabulary)
    THROW (-8);
  return (char *) A (_data_pointer);
}

static char *
make_c_string (Cell c_addr, Cell u)
{
  Cell i;
  Character *string;
  char *name;

  string = (Character *) A (c_addr);
  name = allocate_c_string (sizeof (char) * (u + 1));
  for (i = 0; i < u; i++)
    name[i] = string[i];
  name[i] = 0;
  return name;
}

static Cell
files (Cell u)
{
  return u * sizeof (FILE *);
}

static Cell
_fopen (Cell c_addr, Cell u, Cell fam, Cell fp)
{
  static char *fam_to_mode[6] =
    { "r", "r+", "w", "rb", "rb+", "wb" };
  FILE *f;

  f = fopen (make_c_string (c_addr, u), fam_to_mode[fam]);
  if (f != NULL)
    {
      *(FILE **) A (fp) = f;
      return 0;
    }
  else
    {
      return -1;
    }
}

static Cell
_fclose (Cell fp)
{
  return fclose (*(FILE **) A (fp));
}

static Cell
_fremove (Cell c_addr, Cell u)
{
  return remove (make_c_string (c_addr, u));
}

static Cell
_fgetc (Cell fp)
{
  return fgetc (*(FILE **) A (fp));
}

static Cell
_fputc (Cell c, Cell fp)
{
  return fputc (c, *(FILE **) A (fp));
}

static Cell
_feof (Cell fp)
{
  return feof (*(FILE **) A (fp));
}

static Cell
_ferror (Cell fp)
{
  return ferror (*(FILE **) A (fp));
}

static Cell
_fseek (Cell offset, Cell fp, Cell from)
{
  return fseek (*(FILE **) A (fp), offset * sizeof (Character), from);
}

static Cell
_ftell (Cell fp)
{
  return ftell (*(FILE **) A (fp)) / sizeof (Character);
}

static Cell
f_store (Cell r, Cell f_addr)
{
  return (Cell) (*(Floating *) A (f_addr) = *(Floating *) A (r));
}

static Cell
f_star (Cell r1, Cell r2)
{
  return (Cell) (*(Floating *) A (r2) *= *(Floating *) A (r1));
}

static Cell
f_plus (Cell r1, Cell r2)
{
  return (Cell) (*(Floating *) A (r2) += *(Floating *) A (r1));
}

static Cell
f_minus (Cell r1, Cell r2)
{
  return (Cell) (*(Floating *) A (r2) -= *(Floating *) A (r1));
}

static Cell
f_slash (Cell r1, Cell r2)
{
  return (Cell) (*(Floating *) A (r2) /= *(Floating *) A (r1));
}

static Cell
f_less_than (Cell r1, Cell r2)
{
  return (*(Floating *) A (r1) < *(Floating *) A (r2)) ? -1 : 0;
}

static Cell
floats (Cell n)
{
  return n * sizeof (Floating);
}

static Cell
fnegate (Cell r)
{
  return (Cell) ((*(Floating *) A (r)) = -(*(Floating *) A (r)));
}

static Cell
frot (Cell r)
{
  Floating *r1, *r2, *r3, t;

  r3 = (Floating *) A (r); r2 = r3 - 1; r1 = r2 - 1;
  t = *r3; *r3 = *r1; *r1 = *r2; *r2 = t;
  return 0;
}

static Cell
fswap (Cell r)
{
  Floating *r1, *r2, t;

  r2 = (Floating *) A (r);
  r1 = r2 - 1;
  t = *r2;
  *r2 = *r1;
  return (Cell) (*r1 = t);
}

static Cell
f_zero_less_than (Cell r)
{
  return (*(Floating *) A (r) < 0) ? -1 : 0;
}

static Cell
f_zero_equals (Cell r)
{
  return (*(Floating *) A (r) == 0) ? -1 : 0;
}

static Cell
f_to_s (Cell r)
{
  return (Cell) (*(Floating *) A (r));
}

static Cell
u_to_f (Cell u, Cell r)
{
  (*(Floating *) A (r)) = (Unsigned_Cell) u;
  return 0;
}

static Cell
int_dot (Cell n)
{
  return printf ("%d(%u) ", (int) n, (unsigned) n);
}

static Cell
long_dot (Cell low, Cell high)
{
  long d;

  d = (unsigned) high;
  d <<= sizeof (int) * 8;
  d |= (unsigned) low;
  return printf ("%ld(%lu) ", d, d);
}

static Cell
float_dot (Cell r)
{
  return printf ("%f ", (*(Floating *) A (r)));
}

void
optional (void)
{
  function ("FILES", (Callable) files, 1);
  function ("FOPEN", (Callable) _fopen, 4);
  function ("FCLOSE", (Callable) _fclose, 1);
  function ("FREMOVE", (Callable) _fremove, 2);
  function ("FGETC", (Callable) _fgetc, 1);
  function ("FPUTC", (Callable) _fputc, 2);
  function ("FEOF", (Callable) _feof, 1);
  function ("FERROR", (Callable) _ferror, 1);
  function ("FSEEK", (Callable) _fseek, 3);
  function ("FTELL", (Callable) _ftell, 1);
  routine ("!F!", (Callable) f_store, 2);
  routine ("!F*", (Callable) f_star, 2);
  routine ("!F+", (Callable) f_plus, 2);
  routine ("!F-", (Callable) f_minus, 2);
  routine ("!F/", (Callable) f_slash, 2);
  function ("!F<", (Callable) f_less_than, 2);
  function ("FLOATS", (Callable) floats, 1);
  routine ("!FNEGATE", (Callable) fnegate, 1);
  routine ("!FROT", (Callable) frot, 1);
  routine ("!FSWAP", (Callable) fswap, 1);
  function ("!F0<", (Callable) f_zero_less_than, 1);
  function ("!F0=", (Callable) f_zero_equals, 1);
  function ("F@>S", (Callable) f_to_s, 1);
  routine ("U>!F", (Callable) u_to_f, 2);
  routine ("INT.", (Callable) int_dot, 1);
  routine ("LONG.", (Callable) long_dot, 2);
  routine ("FLOAT@.", (Callable) float_dot, 1);
}
