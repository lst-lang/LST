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
  fill_instruction_word (OP_NOP);
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
macro_key (void)
{
  return getchar ();
}

void
register_core_macros (void)
{
  register_macro ("DP+", (Function) allocate, 1);
  register_macro ("SP-", (Function) static_allocate, 1);
  register_macro ("SLOT,", (Function) macro_emit_instruction_slot, 1);
  register_macro ("INSTRUCTION,", (Function) macro_emit_instruction_word, 1);
  register_macro ("SLOT-INSTRUCTION,",
		  (Function) emit_instruction_slot_and_word, 2);
  register_macro ("FILL,", (Function) macro_fill_instruction_word, 1);
  register_macro ("TAIL-RECURSE,", (Function) macro_tail_recurse, 0);
  register_macro ("RECURSE,", (Function) macro_recurse, 0);
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
  register_macro ("@BLOCK", (Function) macro_fetch_block, 2);
  register_macro ("!BLOCK", (Function) macro_store_block, 2);
}
