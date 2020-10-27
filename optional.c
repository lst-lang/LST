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
#include "execute.h"
#include "optional.h"

typedef float Floating;

static char *
malloc_never_fails (Cell u)
{
  char *p;

  p = malloc (sizeof (char) * u);
  if (p == NULL)
    fatal_error ("MALLOC FAILED");
  return p;
}

static char *
make_c_string (Cell c_addr, Cell u)
{
  Cell i;
  Character *string;
  char *name;

  string = (Character *) A (c_addr);
  name = malloc_never_fails (sizeof (char) * (u + 1));
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
  char *name;
  FILE *f;

  name = make_c_string (c_addr, u);
  f = fopen (name, fam_to_mode[fam]);
  free (name);
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
  int result;
  char *name;

  name = make_c_string (c_addr, u);
  result = remove (name);
  free (name);
  return result;
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
  return fseek (*(FILE **) A (fp),
		offset * sizeof (Character), from);
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
_floor (Cell r)
{
  Cell n;

  n = (Cell) (*(Floating *) A (r));
  (*(Floating *) A (r)) = (Floating) n;
  return n;
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

  r3 = (Floating *) A (r);
  r2 = r3 - 1;
  r1 = r2 - 1;
  t = *r3;
  *r3 = *r1;
  *r1 = *r2;
  return (Cell) (*r2 = t);
}

static Cell
fround (Cell r)
{
  Cell n;
  Floating x;

  x = (*(Floating *) A (r));
  n = (x < 0.0) ? (Cell) (x - 0.5) : (Cell) (x + 0.5);
  (*(Floating *) A (r)) = (Floating) n;
  return n;
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
ud_to_f (Cell l, Cell h, Cell r)
{
  static Unsigned_Cell zero = 0;
  Floating x, fh, fl, fz;

  fh = (Unsigned_Cell) h;
  fl = (Unsigned_Cell) l;
  fz = (Unsigned_Cell) (~zero);
  x = fh * fz + fl;
  (*(Floating *) A (r)) = x;
  return (Cell) x;
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
  return printf ("%ld:%lu ", d, d);
}

static Cell
float_dot (Cell r)
{
  return printf ("%f", (*(Floating *) A (r)));
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
  function ("!F!", (Callable) f_store, 2);
  function ("!F*", (Callable) f_star, 2);
  function ("!F+", (Callable) f_plus, 2);
  function ("!F-", (Callable) f_minus, 2);
  function ("!F/", (Callable) f_slash, 2);
  function ("!F<", (Callable) f_less_than, 2);
  function ("FLOATS", (Callable) floats, 1);
  function ("!FLOOR", (Callable) _floor, 1);
  function ("!FNEGATE", (Callable) fnegate, 1);
  function ("!FROT", (Callable) frot, 1);
  function ("!FROUND", (Callable) fround, 1);
  function ("!FSWAP", (Callable) fswap, 1);
  function ("!F0<", (Callable) f_zero_less_than, 1);
  function ("!F0=", (Callable) f_zero_equals, 1);
  function ("UD>!F", (Callable) ud_to_f, 3);
  routine ("INT.", (Callable) int_dot, 1);
  routine ("LONG.", (Callable) long_dot, 2);
  routine ("FLOAT.", (Callable) float_dot, 1);
}
