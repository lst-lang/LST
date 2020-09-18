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
#include "core.h"
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
  return (fseek (*(FILE **) A (fp),
		offset * sizeof (Character), from)
	  / sizeof (Character));
		
}

Cell
macro_ftell (Cell fp)
{
  return ftell (*(FILE **) A (fp)) / sizeof (Character);
}

Cell
macro_f_store (Cell r, Cell f_addr)
{
  return (Cell) (*(Floating *) A (f_addr) = *(Floating *) A (r));
}

Cell
macro_f_star (Cell r1, Cell r2)
{
  return (Cell) (*(Floating *) A (r2) *= *(Floating *) A (r1));
}

Cell
macro_f_plus (Cell r1, Cell r2)
{
  return (Cell) (*(Floating *) A (r2) += *(Floating *) A (r1));
}

Cell
macro_f_minus (Cell r1, Cell r2)
{
  return (Cell) (*(Floating *) A (r2) -= *(Floating *) A (r1));
}

Cell
macro_f_slash (Cell r1, Cell r2)
{
  return (Cell) (*(Floating *) A (r2) /= *(Floating *) A (r1));
}

Cell
macro_f_less_than (Cell r1, Cell r2)
{
  return (*(Floating *) A (r1) < *(Floating *) A (r2)) ? -1 : 0;
}

Cell
macro_floats (Cell n)
{
  return n * sizeof (Floating);
}

Cell
macro_floor (Cell r)
{
  Cell n;

  n = (Cell) (*(Floating *) A (r));
  (*(Floating *) A (r)) = (Floating) n;
  return n;
}

Cell
macro_fnegate (Cell r)
{
  return (Cell) ((*(Floating *) A (r)) = -(*(Floating *) A (r)));
}

Cell
macro_frot (Cell r)
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

Cell
macro_fround (Cell r)
{
  Cell n;
  Floating x;

  x = (*(Floating *) A (r));
  n = (x < 0.0) ? (Cell) (x - 0.5) : (Cell) (x + 0.5);
  (*(Floating *) A (r)) = (Floating) n;
  return n;
}

Cell
macro_fswap (Cell r)
{
  Floating *r1, *r2, t;

  r2 = (Floating *) A (r);
  r1 = r2 - 1;
  t = *r2;
  *r2 = *r1;
  return (Cell) (*r1 = t);
}

Cell
macro_f_zero_less_than (Cell r)
{
  return (*(Floating *) A (r) < 0) ? -1 : 0;
}

Cell
macro_f_zero_equals (Cell r)
{
  return (*(Floating *) A (r) == 0) ? -1 : 0;
}

Cell
macro_ud_to_f (Cell l, Cell h, Cell r)
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

Cell
macro_dump (Cell addr, Cell u)
{
  unsigned int i, j;
  int *word, *end;

  printf ("\n%-10s%-16s%-12s%-24s%-12s\n",
          "ADDRESS", "BYTECODES", "INTEGER", "ADDRESS", "CHARACTERS");
  word = (int *) A (addr);
  end = (int *) ((char *) word + u);
  while (word < end)
    {
      printf ("%08lx  ", (unsigned long)
	      ((char *) word - (char *) &sys));
      for (i = 0; i < sizeof (int); i++)
        printf ("%02x ", ((unsigned char *) word)[i]);
      printf ("    %-12d", *(int *) word);
      printf ("%-24p", *(void **) word);
      for (i = 0; i < sizeof (int); i++)
        {
          j = ((unsigned char *) word)[i];
          j = isprint (j) ? j : '?';
          printf ("%c", j);
        }
      putchar (10);
      word++;
    }
  return u;
}

Cell
macro_see (void)
{
  static char *opcode_names[] =
    {
     "HALT",
     "NOP", "DUP", "SWAP", "DROP", "OVER",
     "ROT", "MINUS_ROT", "NIP", "TUCK",
     "ROT_DROP", "ROT_DROP_SWAP",
     "PLUS", "MINUS", "STAR", "SLASH",
     "U_PLUS", "U_MINUS", "U_STAR", "U_SLASH",
     "ONE_PLUS", "ONE_MINUS", "INVERT", "AND", "OR", "XOR",
     "TWO_STAR", "U_TWO_SLASH", "TWO_SLASH", "R_SHIFT", "L_SHIFT",
     "TRUE", "FALSE", "ZERO_EQUALS", "ZERO_LESS",
     "U_GREATER_THAN", "U_LESS_THAN", "EQUALS",
     "U_GREATER_EQUALS", "U_LESS_EQUALS", "NOT_EQUALS",
     "GREATER_THAN", "LESS_THAN", "GREATER_EQUALS", "LESS_EQUALS",
     "TO_R", "R_FROM", "R_FETCH", "R_FROM_DROP",
     "FETCH", "STORE", "C_FETCH", "C_STORE", "LIT",
     "JMP", "JZ", "DRJNE", "CALL", "RET", "EX",
     "A", "A_STORE", "FETCH_A", "STORE_A", "FETCH_PLUS", "STORE_PLUS",
     "FETCH_R", "STORE_R",
     "C_FETCH_A", "C_STORE_A", "C_FETCH_PLUS", "C_STORE_PLUS",
     "C_FETCH_R", "C_STORE_R",
     "PICK", "R_PICK", "DEPTH", "MOD", "U_MOD", "NEGATE",
     "THROW", "CATCH", "MACRO", "MACRO",
     "CLEAR_PARAMETER_STACK", "CLEAR_RETURN_STACK", "DOT_S"
    };
  Cell i, *word, *next;
  Entry *e;
  Cell count, in;
  
  in = macro_skip_delimiters (' ');
  count = macro_find_delimiter (' ');
  i = find_word ((Character *) A (in), count);
  if (i == 0)
    THROW (-13);
  printf (": ");
  e = (Entry *) A (i);
  for (i = 0; i < e->name[0]; i++)
    putchar (e->name[i + 1]);
  word = (Cell *) A (e->code_pointer);
  next = word + 1;
  for (;;)
    {
      for (i = 0; i < (Cell) sizeof (Cell); i++)
	switch ((((Unsigned_Cell) *word) >> (i * 8)) & 0xff)
	  {
	  case OP_HALT:
	    printf (" ;");
	    goto ret;
	  case OP_LIT: case OP_JMP: case OP_CALL:
	  case OP_JZ: case OP_MACRO: case _OP_MACRO:
	    printf (" %s",
		    opcode_names[(((Unsigned_Cell) *word) >> (i * 8))
				 & 0xff]);
	    printf (" %d", (int) *next);
	    next++;
	    break;
	  default:
	    printf (" %s",
		    opcode_names[(((Unsigned_Cell) *word) >> (i * 8))
				 & 0xff]);
	    break;
	  }
      word = next;
      next = word + 1;
    }
 ret:
  return 0;
}

Cell
macro_words (void)
{
  int i, offset;
  Entry *e;

  for (offset = sys.task.vocabulary; offset != 0; offset = e->link)
    {
      e = (Entry *) A (offset);
      for (i = 0; i < e->name[0]; i++)
	putchar (e->name[i + 1]);
      putchar (32);
    }
  return 0;
}

Cell
macro_int_dot (Cell n)
{
  return printf ("%d(%u) ", (int) n, (unsigned) n);
}

Cell
macro_long_dot (Cell low, Cell high)
{
  long d;

  d = (unsigned) high;
  d <<= sizeof (int) * 8;
  d |= (unsigned) low;
  return printf ("%ld:%lu ", d, d);
}

Cell
macro_float_dot (Cell r)
{
  return printf ("%f ", (*(Floating *) A (r)));
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
  register_macro ("[F!]", (Function) macro_f_store, 2);
  register_macro ("[F*]", (Function) macro_f_star, 2);
  register_macro ("[F+]", (Function) macro_f_plus, 2);
  register_macro ("[F-]", (Function) macro_f_minus, 2);
  register_macro ("[F/]", (Function) macro_f_slash, 2);
  register_macro ("[F<]", (Function) macro_f_less_than, 2);
  register_macro ("FLOATS", (Function) macro_floats, 1);
  register_macro ("[FLOOR]", (Function) macro_floor, 1);
  register_macro ("[FNEGATE]", (Function) macro_fnegate, 1);
  register_macro ("[FROT]", (Function) macro_frot, 1);
  register_macro ("[FROUND]", (Function) macro_fround, 1);
  register_macro ("[FSWAP]", (Function) macro_fswap, 1);
  register_macro ("[F0<]", (Function) macro_f_zero_less_than, 1);
  register_macro ("[F0=]", (Function) macro_f_zero_equals, 1);
  register_macro ("[UD>F]", (Function) macro_ud_to_f, 3);
  register_macro ("KEY", (Function) macro_key, 0);
  register_macro ("DUMP", (Function) macro_dump, 2);
  register_macro ("SEE", (Function) macro_see, 0);
  register_macro ("WORDS", (Function) macro_words, 0);
  register_macro ("INT.", (Function) macro_int_dot, 1);
  register_macro ("LONG.", (Function) macro_long_dot, 2);
  register_macro ("[FLOAT.]", (Function) macro_float_dot, 1);
}
