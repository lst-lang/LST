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

#define O(v) ((Byte *) (v) - dictionary)
#define S(c) comma_slot (OP_##c)
#define SW(c,w) comma_slot_word (OP_##c, (w))
#define L(w) SW (LIT, w)
#define LO(v) L (O (v))
#define F(c) fill_instruction_word (OP_##c)
#define C(p) compile_call (p)
#define CONSTANT(n) open (); L (n); S (RET)
#define MEMBER_OFFSET(t, m) ((size_t) &((t *)0)->m)

static Cell comma_slot_macro, read_input_macro, skip_macro, stop_macro,
  find_word_macro, parse_number_macro, literal_macro, compile_call_macro;
static Cell last_opcode, last_slot, last_word;

static Cell
comma_slot (Cell slot)
{
  emit_instruction_slot (slot);
  if (slot != OP_NOP)
    {
      last_opcode = slot;
      last_slot = *instruction_slot - 1;
      last_word = *instruction_word;
    }
  return 0;
}

static Cell
comma_word (Cell word)
{
  emit_instruction_word (word);
  return 0;
}

static Cell
comma_slot_word (Cell slot, Cell word)
{
  Cell patch;

  comma_slot (slot);
  patch = *data_pointer;
  comma_word (word);
  return patch;
}

static FILE *
open_boot_file (void)
{
  FILE *boot_file;

  boot_file = fopen ("boot.fs", "r");
  if (boot_file == NULL)
    fatal_error ("BOOT.FS NOT FOUND");
  return boot_file;
}

static Cell
read_character (void)
{
  static FILE *boot_file = NULL;
  Cell c;

  boot_file = (boot_file == NULL) ?
    open_boot_file () : boot_file;
 reget:
  c = fgetc (boot_file);
  if (feof (boot_file))
    {
      if (boot_file == stdin)
	{
	  return 0;
	}
      else
	{
	  boot_file = stdin;
	  goto reget;
	}
    }
  else
    {
      return c;
    }
}

static Cell
read_input (void)
{
  Character *buffer;
  Cell n, c;

  buffer = (Character *) A (*input_buffer);
  for (n = 0; n < BUFFER_SIZE; n++)
    if ((c = read_character ()) == 0 || c == '\n')
      break;
    else
      buffer[n] = c;
  *number_input_buffer = (buffer[0] == '\\') ? 0 : n;
  return *in = 0;
}

static Cell
skip (Cell c)
{
  Character *buffer;

  buffer = (Character *) A (*input_buffer);
  while (*in < *number_input_buffer
	 && (buffer[*in] == c || (c == ' ' && isspace (buffer[*in]))))
    (*in)++;
  return *in;
}

static Cell
stop (Cell c)
{
  Character *buffer;

  buffer = (Character *) A (*input_buffer);
  while (*in < *number_input_buffer
	 && !(buffer[*in] == c || (c == ' ' && isspace (buffer[*in]))))
    (*in)++;
  return *in;
}

static Cell
_find_word (Cell c_addr, Cell u)
{
  return find_word ((Character *) A (c_addr), u);
}

static void
__define (void)
{
  Cell n, l;

  n = skip (' ');
  l = stop (' ') - n;
  F (NOP);
  define ((Character *) A (*input_buffer) + n, l, *vocabulary);
}

static Cell
_define (void)
{
  *state = 0;
  __define ();
  return 0;
}

static Cell
_return (void)
{
  if (last_opcode == OP_CALL)
    {
      Cell offset, *word_pointer;

      offset = sizeof (Cell) - (last_word - last_slot);
      word_pointer = (Cell *) (dictionary + last_word - sizeof (Cell));
      *word_pointer &= ~MASK_SLOT (0xff, offset);
      *word_pointer |= MASK_SLOT (OP_JMP, offset);
      last_opcode = OP_JMP;
    }
  else
    {
      S (RET);
    }
  return 0;
}

static Cell
immediate (void)
{
  ((Entry *) A (*vocabulary))->flag = 1;
  return 0;
}

static Cell
left_bracket (void)
{
  *state = 1;
  return 0;
}

static Cell
right_bracket (void)
{
  *state = 0;
  return 0;
}

static Cell
_if (void)
{
  return SW (JZ, -1);
}

static Cell
then (Cell patch)
{
  F (NOP);
  V (patch) = *data_pointer;
  return 0;
}

static Cell
comma_fill_nop (void)
{
  fill_instruction_word (OP_NOP);
  return 0;
}

static Cell
parse_number (Cell c_addr, Cell u)
{
  Character *name;

  name = (Character *) A (c_addr);
  if (u == 3 && name[0] == '\'' && name[2] == '\'')
    {
      return name[1];
    }
  else
    {
      int n, sign;

      sign = 1;
      if (*name == '-')
	{
	  sign = -1;
	  name++, u--;
	}
      else
	{
	  sign = 1;
	}

      for (n = 0; u > 0; u--)
	if (!isdigit (*name))
	  fatal_error ("BAD NUMBER");
	else
	  n = n * 10 + *name++ - '0';
      return n * sign;
    }
}

static Cell
literal (Cell n)
{
  SW (LIT, n);
  return 0;
}

static Cell
compile_call (Cell n)
{
  SW (CALL, n);
  F (NOP);
  return 0;
}

static Cell
open (void)
{
  __define ();
  return *vocabulary;
}

static Cell
if_end (void)
{
  LO (in); S (FETCH); LO (number_input_buffer); S (FETCH);
  S (EQUALS); return _if ();
}

static Cell
if_not_found (void)
{
  L (' '); C (skip_macro); LO (input_buffer);
  S (FETCH); S (OVER); S (PLUS); S (SWAP); L (' ');
  C (stop_macro); S (SWAP); S (MINUS);
  S (OVER); S (OVER); C (find_word_macro); S (DUP);
  S (ZERO_EQUALS); return _if ();
}

static void
do_word (Cell next)
{
  Cell interpreting;

  S (R_FETCH); S (R_FROM); L (MEMBER_OFFSET (Entry, code_pointer));
  S (PLUS); S (FETCH); S (TO_R); L (MEMBER_OFFSET (Entry, flag));
  S (PLUS); S (FETCH); LO (state); S (FETCH); S (OR);
  interpreting = _if (); S (DROP); S (EX); F (NOP); SW (JMP, next);
  then (interpreting); S (DROP); S (R_FROM); C (compile_call_macro);
  SW (JMP, next);
}

static void
do_number (Cell next)
{
  Cell interpreting;

  C (parse_number_macro); LO (state); S (FETCH);
  interpreting = _if (); S (DROP); SW (JMP, next);
  then (interpreting); S (DROP); C (literal_macro);
  SW (JMP, next);
}

static void
parse_word (Cell next)
{
  Cell word_or_number;
  
  word_or_number = if_not_found (); S (DROP); S (DROP);
  do_number (next); then (word_or_number); S (DROP);
  S (TO_R); S (DROP); S (DROP); do_word (next);
}

static void
interpret (void)
{
  Cell tail, next, end_of_line;

  F (NOP); tail = *data_pointer; C (read_input_macro); F (NOP);
  next = *data_pointer; end_of_line = if_end (); S (DROP);
  SW (JMP, tail); then (end_of_line); S (DROP); parse_word (next);
}

static Cell
declare (void)
{
  static Cell count = 0;
  Cell declares[] =
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      sizeof (Cell), sizeof (Character)
    };

  declares[0] = O (terminal_input_buffer);
  declares[1] = BUFFER_SIZE;
  declares[2] = O (input_buffer);
  declares[3] = O (number_input_buffer);
  declares[4] = O (in);
  declares[5] = O (state);
  declares[6] = O (data_pointer);
  declares[7] = O (exception_handler);
  declares[8] = MEMBER_OFFSET (Entry, flag);
  declares[9] = MEMBER_OFFSET (Entry, code_pointer);
  declares[10] = MEMBER_OFFSET (Entry, parameter);
  declares[11] = (Cell) ((((Unsigned_Cell) 1) << (sizeof (Cell) * 8 - 1)));
  CONSTANT (declares[count++]);
  return 0;
}

static Cell
allot (Cell n)
{
  allocate (n);
  return 0;
}

static Cell
key (void)
{
  return getc (stdin);
}

static Cell
emit (Cell c)
{
  putc (c, stdout);
  return 0;
}

static Cell
primitive (Cell n)
{
  Cell interpreting;

  __define (); LO (state); S (FETCH); interpreting = _if ();
  S (DROP); comma_slot (n); _return (); then (interpreting);
  S (DROP); L (n); C (comma_slot_macro); _return (); immediate ();
  return n + 1;
}

static Cell
opcode (Cell n)
{
  CONSTANT (n);
  return n + 1;
}

static Cell
fetch_block (Cell b, Cell u)
{
  FILE *f;
  char name[32];

  sprintf (name, "b%07u.blk", (unsigned) (b / 100));
  b %= 100;
  f = fopen (name, "r");
  if (f == NULL)
    {
      THROW (-35);
      return 0;
    }
  else if (fseek (f, b * 1024 * sizeof (Character), SEEK_SET) != 0)
    {
      fclose (f);
      THROW (-35);
      return 0;
    }
  else if (fread ((char *) A (u), sizeof (Character), 1024, f) != 1024)
    {
      fclose (f);
      THROW (-33);
      return 0;
    }
  else
    {
      fclose (f);
      return u;
    }
}

static Cell
store_block (Cell b, Cell u)
{
  FILE *f;
  char name[32];

  sprintf (name, "b%07u.blk", (unsigned) (b / 100));
  b %= 100;
  f = fopen (name, "r+");
  if (f == NULL)
    {
      THROW (-35);
      return 0;
    }
  else if (fseek (f, b * 1024 * sizeof (Character), SEEK_SET) != 0)
    {
      fclose (f);
      THROW (-35);
      return 0;
    }
  else if (fwrite ((char *) A (u), sizeof (Character), 1024, f) != 1024)
    {
      fclose (f);
      THROW (-34);
      return 0;
    }
  else
    {
      fclose (f);
      return u;
    }
}

static void
core (void)
{
  routine (",WORD", (Callable) comma_word, 1);
  function (",SLOT-WORD", (Callable) comma_slot_word, 2);
  routine (",FILL-NOP", (Callable) comma_fill_nop, 0);
  routine ("IMMEDIATE", (Callable) immediate, 0); immediate ();
  routine ("[", (Callable) left_bracket, 0); immediate ();
  routine ("]", (Callable) right_bracket, 0);
  routine (":", (Callable) _define, 0); immediate ();
  routine (";", (Callable) _return, 0); immediate ();
  function ("IF", (Callable) _if, 0); immediate ();
  routine ("THEN", (Callable) then, 1); immediate ();
  function ("OPEN", (Callable) open, 0);
  routine ("DECLARE", (Callable) declare, 0);
  routine ("ALLOT", (Callable) allot, 1);
  function ("KEY", (Callable) key, 0);
  routine ("EMIT", (Callable) emit, 1);
  function (">>", (Callable) primitive, 1);
  function ("|", (Callable) opcode, 1);
  comma_slot_macro = routine (",SLOT", (Callable) comma_slot, 1);
  read_input_macro = routine ("READ-INPUT", (Callable) read_input, 0);
  skip_macro = function ("SKIP", (Callable) skip, 1);
  stop_macro = function ("STOP", (Callable) stop, 1);
  find_word_macro = function ("FIND-WORD", (Callable) _find_word, 2);
  parse_number_macro = function ("PARSE-NUMBER", (Callable) parse_number, 2);
  literal_macro = routine ("LITERAL", (Callable) literal, 1); immediate ();
  compile_call_macro = routine (",CALL", (Callable) compile_call, 1);
  function ("@BLOCK", (Callable) fetch_block, 2);
  function ("!BLOCK", (Callable) store_block, 2);
}

static Cell
driver (void)
{
  Cell _driver;

  F (NOP); _driver = *data_pointer;
  S (CLEAR_PARAMETERS); S (CLEAR_RETURNS); interpret ();
  return _driver;
}

void
boot (void)
{
  if (CATCH != 0)
    fatal_error ("BOOT FAILED");
  reset_system ();
  core ();
  optional ();
  execute (driver ());
}
