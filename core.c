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

#define O(m)								\
  MEMBER_OFFSET (System, task) + MEMBER_OFFSET (Terminal_Task, m)

static char *error_messages[] =
  {
   "ABORT",
   "ABORT\"",
   "STACK OVERFLOW",
   "STACK UNDERFLOW",
   "RETURN STACK OVERFLOW",
   "RETURN STACK UNDERFLOW",
   "DO-LOOPS NESTED TOO DEEPLY DURING EXECUTION",
   "DICTIONARY OVERFLOW",
   "INVALID MEMORY ADDRESS",
   "DIVISION BY ZERO",
   "RESULT OUT OF RANGE",
   "ARGUMENT TYPE MISMATCH",
   "UNDEFINED WORD",
   "INTERPRETING A COMPILE-ONLY WORD",
   "INVALID FORGET",
   "ATTEMPT TO USE ZERO-LENGTH STRING AS A NAME",
   "PICTURED NUMERIC OUTPUT STRING OVERFLOW",
   "PARSED STRING OVERFLOW",
   "DEFINITION NAME TOO LONG",
   "WRITE TO A READ-ONLY LOCATION",
   "UNSUPPORTED OPERATION",
   "CONTROL STRUCTURE MISMATCH",
   "ADDRESS ALIGNMENT EXCEPTION",
   "INVALID NUMERIC ARGUMENT",
   "RETURN STACK IMBALANCE",
   "LOOP PARAMETERS UNAVAILABLE",
   "INVALID RECURSION",
   "USER INTERRUPT",
   "COMPILER NESTING",
   "OBSOLESCENT FEATURE",
   ">BODY USED ON NON-CREATED DEFINITION",
   "INVALID NAME ARGUMENT",
   "BLOCK READ EXCEPTION",
   "BLOCK WRITE EXCEPTION",
   "INVALID BLOCK NUMBER",
   "INVALID FILE POSITION",
   "FILE I/O EXCEPTION",
   "NON-EXISTENT FILE",
   "UNEXPECTED END OF FILE",
   "INVALIDBASEFOR FLOATING POINT CONVERSION",
   "LOSS OF PRECISION",
   "FLOATING-POINT DIVIDE BY ZERO",
   "FLOATING-POINT RESULT OUT OF RANGE",
   "FLOATING-POINT STACK OVERFLOW",
   "FLOATING-POINT STACK UNDERFLOW",
   "FLOATING-POINT INVALID ARGUMENT",
   "COMPILATION WORD LIST DELETED",
   "INVALID POSTPONE",
   "SEARCH-ORDER OVERFLOW",
   "SEARCH-ORDER UNDERFLOW",
   "COMPILATION WORD LIST CHANGED",
   "CONTROL-FLOW STACK OVERFLOW",
   "EXCEPTION STACK OVERFLOW",
   "FLOATING-POINT UNDERFLOW",
   "FLOATING-POINT UNIDENTIFIED FAULT",
   "QUIT",
   "EXCEPTION IN SENDING OR RECEIVING A CHARACTER",
   "[IF], [ELSE], OR [THEN] EXCEPTION",
   "ALLOCATE",
   "FREE",
   "RESIZE",
   "CLOSE-FILE",
   "CREATE-FILE",
   "DELETE-FILE",
   "FILE-POSITION",
   "FILE-SIZE",
   "FILE-STATUS",
   "FLUSH-FILE",
   "OPEN-FILE",
   "READ-FILE",
   "READ-LINE",
   "RENAME-FILE",
   "REPOSITION-FILE",
   "RESIZE-FILE",
   "WRITE-FILE",
   "WRITE-LINE",
   "MALFORMED XCHAR",
   "SUBSTITUTE",
   "REPLACES"
  };

static int
is_delimiter (Character c, Character d)
{
  return ((c == d) || (isspace (c) && d == ' '));
}

Cell
macro_emit_instruction_slot (Cell opcode)
{
  emit_instruction_slot (opcode);
  return opcode;
}

Cell
macro_emit_instruction_word (Cell instruction)
{
  emit_instruction_word (instruction);
  return instruction;
}

Cell
macro_fill_instruction_word (Cell opcode)
{
  fill_instruction_word (opcode);
  return opcode;
}

Cell
macro_tail_recurse (void)
{
  emit_instruction_slot_and_word (OP_JMP, sys.task.recursive);
  return sys.task.vocabulary;
}

Cell
macro_recurse (void)
{
  emit_instruction_slot_and_word (OP_CALL, sys.task.recursive);
  return sys.task.vocabulary;
}

Cell macro_skip_delimiters (Cell);
Cell macro_find_delimiter (Cell);
Cell
macro_define (Cell c_addr, Cell u)
{
  if (u == -1)
    {
      Cell count, in;

      in = macro_skip_delimiters (' ');
      count = macro_find_delimiter (' ');
      return word_begin ((Character *) A (in), count);
    }
  else
    {
      return word_begin ((Character *) A (c_addr), u);
    }
}

Cell
macro_end_define (Cell offset)
{
  emit_instruction_slot (OP_HALT);
  word_end (offset);
  return offset;
}

Cell
macro_find_word (Cell c_addr, Cell u)
{
  return find_word ((Character *) A (c_addr), u);
}

Cell
macro_accept (Cell c_addr, Cell u)
{
  Character c, *string;
  Cell length;

  fflush (stdout);
  string = (Character *) A (c_addr);
  for (length = 0; length < u; length++)
    if ((c = getchar ()) != '\n')
      *string++ = c;
    else
      break;
  return length;
}

Cell
macro_skip_delimiters (Cell c)
{
  Cell in, end;
  Character *buffer;

  buffer = (Character *) A (sys.task.input_buffer);
  end = sys.task.number_input_buffer;
  in = sys.task.in;
  while (in < end && is_delimiter (buffer[in], c))
    in++;
  sys.task.in = in;
  return sys.task.input_buffer + in;
}

Cell
macro_find_delimiter (Cell c)
{
  Cell count, in, end;
  Character *buffer;

  buffer = (Character *) A (sys.task.input_buffer);
  end = sys.task.number_input_buffer;
  count = 0;
  for (in = sys.task.in; in < end; in++)
    {
      if (is_delimiter (buffer[in], c))
	break;
      count++;
    }
  sys.task.in = in;
  return count;
}

Cell
macro_word (Cell c)
{
  Cell count, in, i;
  Character *buffer, *output;

  in = macro_skip_delimiters (c);
  count = macro_find_delimiter (c);
  output = sys.task.pictured_string;
  buffer = (Character *) A (in);
  if (count > 31)
    count = 31;
  for (i = 1; i < count + 1; i++)
    output[i] = buffer[i - 1];
  output[0] = i - 1;
  return O (pictured_string);
}

Cell
macro_parse_digit (Cell c)
{
  Cell digit;

  digit = 0;
  if (c <= '9' && c >= '0')
    digit = c - '0';
  else
    digit = c - 'A' + 10;
  return digit;
}

Cell
macro_parse_number (Cell c_addr, Cell u)
{
  Character *buffer;
  Cell i, digit, number;

  buffer = (Character *) A (c_addr);
  number = 0;
  for (i = 0; i < u; i++)
    {
      digit = macro_parse_digit (buffer[i]);
      if (digit < 0 || digit >= sys.task.base)
	{
	  THROW (-13);
	}
      else
	{
	  number *= sys.task.base;
	  number += digit;
	}
    }
  return number;
}

Cell
macro_number (void)
{
  Cell i, size, number, sign;
  Character *output;

  sign = 1;
  output = (Character *) A (macro_word (' '));
  number = 0;
  size = *output + 1;
  for (i = 1; i < size; i++)
    if (output[i] == '-' && i == 1)
      {
	sign = -1;
      }
    else if (!isdigit (output[i]))
      {
	break;
      }
    else
      {
	number *= 10;
	number += output[i] - '0';
      }
  return number * sign;
}

Cell
macro_immediate (void)
{
  Entry *e;

  e = (Entry *) A (sys.task.vocabulary);
  e->flag = 1;
  return sys.task.vocabulary;
}

Cell
macro_literal (Cell number)
{
  emit_instruction_slot_and_word (OP_LIT, number);
  return number;
}

Cell
macro_emit (Cell number)
{
  putchar (number);
  return number;
}

Cell
macro_erase (Cell addr, Cell u)
{
  memset((char *) A (addr), 0, u);
  return u;
}

Cell
macro_fetch_block (Cell b, Cell u)
{
  FILE *f;
  char name[32];

  sprintf (name, "b%05u.blk", (unsigned) (b / 100));
  b %= 100;
  f = fopen (name, "r");
  if (f == NULL)
    {
      THROW (-35);
      return 0;
    }
  else if (fseek (f, b * 1024 * sizeof (Character),
		  SEEK_SET) != 0)
    {
      fclose (f);
      THROW (-35);
      return 0;
    }
  else if (fread ((char *) A (u), sizeof (Character),
		  1024, f) != 1024)
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

Cell
macro_store_block (Cell b, Cell u)
{
  FILE *f;
  char name[32];

  sprintf (name, "b%05u.blk", (unsigned) (b / 100));
  b %= 100;
  f = fopen (name, "r+");
  if (f == NULL)
    {
      THROW (-35);
      return 0;
    }
  else if (fseek (f, b * 1024 * sizeof (Character),
		  SEEK_SET) != 0)
    {
      fclose (f);
      THROW (-35);
      return 0;
    }
  else if (fwrite ((char *) A (u), sizeof (Character),
		   1024, f) != 1024)
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

Cell
macro_move (Cell addr1, Cell addr2, Cell u)
{
  if (u)
    {
      Cell *_addr1, *_addr2;
      
      _addr1 = (Cell *) A (addr1);
      _addr2 = (Cell *) A (addr2);
      while (u--)
	*_addr2++ = *_addr1++;
      return 1;
    }
  else
    {
      return 0;
    }
}

Cell
macro_cmove (Cell c_addr1, Cell c_addr2, Cell u)
{
  if (u)
    {
      Character *_c_addr1, *_c_addr2;
      
      _c_addr1 = (Character *) A (c_addr1);
      _c_addr2 = (Character *) A (c_addr2);
      while (u--)
	*_c_addr2++ = *_c_addr1++;
      return 1;
    }
  else
    {
      return 0;
    }
}

Cell
macro_cmove_up (Cell c_addr1, Cell c_addr2, Cell u)
{
  if (u)
    {
      Character *_c_addr1, *_c_addr2;
      
      _c_addr1 = ((Character *) A (c_addr1)) + u;
      _c_addr2 = ((Character *) A (c_addr2)) + u;
      while (u--)
	*--_c_addr2 = *--_c_addr1;
      return 1;
    }
  else
    {
      return 0;
    }
}

Cell
macro_compare (Cell c_addr1, Cell u1, Cell c_addr2, Cell u2)
{
  Cell i, u;
  Character *_c_addr1, *_c_addr2, a, b;

  u = (u1 > u2) ? u2 : u1;
  _c_addr1 = (Character *) A (c_addr1);
  _c_addr2 = (Character *) A (c_addr2);
  for (i = 0; i < u; ++i)
    {
      a = _c_addr1[i];
      b = _c_addr2[i];
      if (a > b)
	return 1;
      else if (a < b)
	return -1;
      else
	continue;
    }
  return (u1 < u2) ? -1 : (u1 > u2);
}

Cell
macro_display_syserr (Cell n)
{
  n = (n < 0) ? -n : n;
  if (n == 13)
    {
      Cell i;
      Character *name;

      name = (Character *) A (sys.task.last_word);
      for (i = 0; i < sys.task.last_word_size; i++)
	putchar (toupper (name[i]));
      printf ("?\n");
    }
  else
    {
      n -= 1;
      printf ("%s!\n", error_messages[n]);
    }
  return n;
}

Cell
macro_align_number (Cell n, Cell a)
{
  return ((n + a - 1) & ~(a - 1));
}

Cell
macro_key (void)
{
  fflush (stdout);
  return getchar ();
}

void
register_core_macros (void)
{
  register_macro ("ALLOT", (Function) allocate, 1);
  register_macro ("STATIC-ALLOT", (Function) static_allocate, 1);
  register_macro ("SLOT,", (Function) macro_emit_instruction_slot, 1);
  register_macro ("INSTRUCTION,", (Function) macro_emit_instruction_word, 1);
  register_macro ("SLOT-INSTRUCTION,",
		  (Function) emit_instruction_slot_and_word, 2);
  register_macro ("FILL,", (Function) macro_fill_instruction_word, 1);
  register_macro ("TAIL-RECURSE,", (Function) macro_tail_recurse, 0);
  register_macro ("BEGIN,", (Function) macro_define, 2);
  register_macro ("END,", (Function) macro_end_define, 1);
  register_macro ("FIND-WORD", (Function) macro_find_word, 2);
  register_macro ("ACCEPT", (Function) macro_accept, 2);
  register_macro ("SKIP-DELIMITERS", (Function) macro_skip_delimiters, 1);
  register_macro ("FIND-DELIMITER", (Function) macro_find_delimiter, 1);
  register_macro ("WORD", (Function) macro_word, 1);
  register_macro ("PARSE-DIGIT", (Function) macro_parse_digit, 1);
  register_macro ("PARSE-NUMBER", (Function) macro_parse_number, 2);
  register_macro ("NUMBER", (Function) macro_number, 0);
  register_macro ("IMMEDIATE", (Function) macro_immediate, 0);
  register_macro ("LITERAL", (Function) macro_literal, 1);
  register_macro ("EMIT", (Function) macro_emit, 1);
  register_macro ("ERASE", (Function) macro_erase, 2);
  register_macro ("@BLOCK", (Function) macro_fetch_block, 2);
  register_macro ("!BLOCK", (Function) macro_store_block, 2);
  register_macro ("RECURSE", (Function) macro_recurse, 0);
  register_macro ("COMPARE", (Function) macro_compare, 4);
  register_macro ("MOVE", (Function) macro_move, 3);
  register_macro ("CMOVE", (Function) macro_cmove, 3);
  register_macro ("CMOVE>", (Function) macro_cmove_up, 3);
  register_macro ("DISPLAY-SYSERR", (Function) macro_display_syserr, 1);
  register_macro ("ALIGN-NUMBER", (Function) macro_align_number, 2);
}
