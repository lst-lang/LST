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
#include "execute.h"
#include "macro.h"
#include "core.h"

#define O(m)								\
  MEMBER_OFFSET (System, task) + MEMBER_OFFSET (Terminal_Task, m)

static Character *
extract_name (char *name)
{
  static Character buffer[ENTRY_NAME_SIZE];
  Cell i, j, length;

  length = strlen (name);
  for (i = 0, j = 1; i < length; i++)
    if (j < ENTRY_NAME_SIZE)
      buffer[j++] = name[i];
  buffer[0] = j - 1;
  return buffer;
}

static void
define (char *name)
{
  Character *temp;

  temp = extract_name (name);
  word_end (word_begin (temp + 1, temp[0]));
}

static void
_emit_call (Character *name, Cell size)
{
  Cell entry_offset;

  entry_offset = find_word (name, size);
  if (entry_offset == 0)
    {
      while (*name)
	fputc (*name++, stderr);
      fprintf (stderr, "?\n");
      fatal_error("WORD NOT FOUND");
    }
  else
    {
      Entry *e;

      e = (Entry *) A (entry_offset);
      emit_slot_with_instruction (OP_CALL, e->code_pointer);
      fill_instruction (OP_NOP);
    }
}

static void
emit_call (char *name)
{
  Character *temp;

  temp = extract_name (name);
  _emit_call (temp + 1, temp[0]);
}

static Cell
emit_number (void)
{
  Cell number;

  number = macro_number ();
  emit_slot_with_instruction (OP_LIT, number);
  return number;
}

static Cell
postpone (void)
{
  Cell count, in;
  
  in = macro_skip_delimiters (' ');
  count = macro_find_delimiter (' ');
  _emit_call ((Character *) A (in), count);  
  return 0;
}

static int
read_line (Character *output, Cell size, FILE *f)
{
  Cell i;
  Character c;

  c = 0;
  for (i = 0; i < 64; i++)
    {
      c = fgetc (f);
      if (feof (f))
	return -1;
      else if (c == '\n')
	break;
      else if (i < size)
	output[i] = c;
      else
	break;
    }
  return (output[0] == '\\') ? 0 : i;
}

static Cell
accept_input (Cell c_addr, Cell u)
{
  static Cell loaded = 0;
  static FILE *bootstrap_file = NULL;
  Character *output;

  output = (Character *) A (c_addr);
  if (!loaded)
    {
      if (bootstrap_file == NULL)
	{
	  bootstrap_file = fopen ("bootstrp.blk", "r");
	  if (bootstrap_file == NULL)
	    fatal_error ("BOOTSTRP.BLK NOT FOUND");
	}
      u = read_line (output, u, bootstrap_file);
      if (u == -1)
	{
	  fclose (bootstrap_file);
	  bootstrap_file = NULL;
	  loaded = 1;
	  return 0;
	}
      else
	{
	  return u;
	}
    }
  else
    {
      return macro_accept (c_addr, u);
    }
}

static void
define_macro_word (char *macro_name, Cell drop_result)
{
  Cell name;

  define (macro_name);
  name = sys.task.vocabulary + MEMBER_OFFSET (Entry, name);
  emit_slot_with_instruction (OP_MACRO, name);
  if (drop_result)
    emit_slot (OP_DROP);
  emit_slot (OP_RET);
}

static void
define_code_word (char *name, Opcode code)
{
  define (name);
  emit_slot_with_instruction (OP_LIT, code);
  emit_call ("SLOT,");
  emit_slot (OP_RET);
}

static void
define_constant_word (char *name, Cell value)
{
  define (name);
  emit_slot_with_instruction (OP_LIT, value);
  emit_slot (OP_RET);
}

static void
define_words (void)
{
  define_macro_word ("SLOT,", 1);
  define_macro_word ("INSTRUCTION,", 1);
  define_macro_word ("SLOT-INSTRUCTION,", 0);
  define_macro_word ("FILL,", 1);
  define_macro_word ("TAIL-RECURSE,", 1);
  define_macro_word ("BEGIN,", 0);
  define_macro_word ("END,", 1);
  define_macro_word ("POSTPONE,", 1);
  define_macro_word ("NUMBER,", 1);
  define_macro_word ("FIND-WORD", 0);
  define_macro_word ("ALLOT", 0);
  define_macro_word ("STATIC-ALLOT", 0);
  define_macro_word ("ACCEPT", 0);
  define_macro_word ("ACCEPT-INPUT", 0);
  define_macro_word ("SKIP-DELIMITERS", 0);
  define_macro_word ("FIND-DELIMITER", 0);
  define_macro_word ("PARSE-DIGIT", 0);
  define_macro_word ("PARSE-NUMBER", 0);
  define_macro_word ("NUMBER", 0);
  define_macro_word ("WORD", 0);
  define_macro_word ("IMMEDIATE", 1);
  define_macro_word ("LITERAL", 1);
  define_macro_word ("EMIT", 1);
  define_macro_word ("ERASE", 1);
  define_macro_word ("!BLOCK", 0);
  define_macro_word ("@BLOCK", 0);
  define_macro_word ("RECURSE", 1);
  define_macro_word ("COMPARE", 0);
  define_macro_word ("MOVE", 1);
  define_macro_word ("CMOVE", 1);
  define_macro_word ("CMOVE>", 1);
  define_macro_word ("DISPLAY-SYSERR", 1);
  define_macro_word ("ALIGN-NUMBER", 0);
  define_macro_word ("KEY", 0);
  define_macro_word ("DUMP", 1);
  define_macro_word ("SEE", 1);
  define_macro_word ("WORDS", 1);
  define_macro_word ("INT.", 1);
  define_macro_word ("LONG.", 1);
				  
  define_code_word ("HALT,", OP_HALT);
  define_code_word ("NOP,", OP_NOP);
  define_code_word ("DUP,", OP_DUP);
  define_code_word ("SWAP,", OP_SWAP);
  define_code_word ("DROP,", OP_DROP);
  define_code_word ("OVER,", OP_OVER);
  define_code_word ("ROT,", OP_ROT);
  define_code_word ("MINUS-ROT,", OP_MINUS_ROT);
  define_code_word ("NIP,", OP_NIP);
  define_code_word ("TUCK,", OP_TUCK);
  define_code_word ("ROT-DROP,", OP_ROT_DROP);
  define_code_word ("ROT-DROP-SWAP,", OP_ROT_DROP_SWAP);
  define_code_word ("PLUS,", OP_PLUS);
  define_code_word ("MINUS,", OP_MINUS);
  define_code_word ("STAR,", OP_STAR);
  define_code_word ("SLASH,", OP_SLASH);
  define_code_word ("U-PLUS,", OP_U_PLUS);
  define_code_word ("U-MINUS,", OP_U_MINUS);
  define_code_word ("U-STAR,", OP_U_STAR);
  define_code_word ("U-SLASH,", OP_U_SLASH);
  define_code_word ("ONE-PLUS,", OP_ONE_PLUS);
  define_code_word ("ONE-MINUS,", OP_ONE_MINUS);
  define_code_word ("INVERT,", OP_INVERT);
  define_code_word ("AND,", OP_AND);
  define_code_word ("OR,", OP_OR);
  define_code_word ("XOR,", OP_XOR);
  define_code_word ("TWO-STAR,", OP_TWO_STAR);
  define_code_word ("U-TWO-SLASH,", OP_U_TWO_SLASH);
  define_code_word ("TWO-SLASH,", OP_TWO_SLASH);
  define_code_word ("R-SHIFT,", OP_R_SHIFT);
  define_code_word ("L-SHIFT,", OP_L_SHIFT);
  define_code_word ("TRUE,", OP_TRUE);
  define_code_word ("FALSE,", OP_FALSE);
  define_code_word ("ZERO-EQUALS,", OP_ZERO_EQUALS);
  define_code_word ("ZERO-LESS,", OP_ZERO_LESS);
  define_code_word ("U-GREATER-THAN,", OP_U_GREATER_THAN);
  define_code_word ("U-LESS-THAN,", OP_U_LESS_THAN);
  define_code_word ("EQUALS,", OP_EQUALS);
  define_code_word ("U-GREATER-EQUALS,", OP_U_GREATER_EQUALS);
  define_code_word ("U-LESS-EQUALS,", OP_U_LESS_EQUALS);
  define_code_word ("NOT-EQUALS,", OP_NOT_EQUALS);
  define_code_word ("GREATER-THAN,", OP_GREATER_THAN);
  define_code_word ("LESS-THAN,", OP_LESS_THAN);
  define_code_word ("GREATER-EQUALS,", OP_GREATER_EQUALS);
  define_code_word ("LESS-EQUALS,", OP_LESS_EQUALS);
  define_code_word ("TO-R,", OP_TO_R);
  define_code_word ("R-FROM,", OP_R_FROM);
  define_code_word ("R-FETCH,", OP_R_FETCH);
  define_code_word ("R-FROM-DROP,", OP_R_FROM_DROP);
  define_code_word ("FETCH,", OP_FETCH);
  define_code_word ("STORE,", OP_STORE);
  define_code_word ("C-FETCH,", OP_C_FETCH);
  define_code_word ("C-STORE,", OP_C_STORE);
  define_code_word ("LIT,", OP_LIT);
  define_code_word ("JMP,", OP_JMP);
  define_code_word ("JZ,", OP_JZ);
  define_code_word ("DRJNE,", OP_DRJNE);
  define_code_word ("CALL,", OP_CALL);
  define_code_word ("RET,", OP_RET);
  define_code_word ("EX,", OP_EX);
  define_code_word ("A,", OP_A);
  define_code_word ("A-STORE,", OP_A_STORE);
  define_code_word ("FETCH-A,", OP_FETCH_A);
  define_code_word ("STORE-A,", OP_STORE_A);
  define_code_word ("FETCH-PLUS,", OP_FETCH_PLUS);
  define_code_word ("STORE-PLUS,", OP_STORE_PLUS);
  define_code_word ("FETCH-R,", OP_FETCH_R);
  define_code_word ("STORE-R,", OP_STORE_R);
  define_code_word ("C-FETCH-A,", OP_C_FETCH_A);
  define_code_word ("C-STORE-A,", OP_C_STORE_A);
  define_code_word ("C-FETCH-PLUS,", OP_C_FETCH_PLUS);
  define_code_word ("C-STORE-PLUS,", OP_C_STORE_PLUS);
  define_code_word ("C-FETCH-R,", OP_C_FETCH_R);
  define_code_word ("C-STORE-R,", OP_C_STORE_R);
  define_code_word ("PICK,", OP_PICK);
  define_code_word ("RPICK,", OP_R_PICK);
  define_code_word ("DEPTH,", OP_DEPTH);
  define_code_word ("MOD,", OP_MOD);
  define_code_word ("U-MOD,", OP_U_MOD);
  define_code_word ("NEGATE,", OP_NEGATE);
  define_code_word ("THROW,", OP_THROW);
  define_code_word ("CATCH,", OP_CATCH);
  define_code_word ("MACRO,", OP_MACRO);
  define_code_word ("CLEAR-PARAMETER-STACK,", OP_CLEAR_PARAMETER_STACK);
  define_code_word ("CLEAR-RETURN-STACK,", OP_CLEAR_RETURN_STACK);
  define_code_word ("DOT-S,", OP_DOT_S);

  define_constant_word ("ENTRY-NAME", MEMBER_OFFSET (Entry, name));
  define_constant_word ("ENTRY-FLAG", MEMBER_OFFSET (Entry, flag));
  define_constant_word ("ENTRY-CODE", MEMBER_OFFSET (Entry, code_pointer));
  define_constant_word ("ENTRY-PARAMETER",
			MEMBER_OFFSET (Entry, parameter_field));
  define_constant_word ("BASE", O (base));
  define_constant_word ("IN", O (in));
  define_constant_word ("STATE", O (state));
  define_constant_word ("TIB", O (terminal_input_buffer));
  define_constant_word ("INPUT", O (input_buffer));
  define_constant_word ("#INPUT", O (number_input_buffer));
  define_constant_word ("/TIB", BUFFER_SIZE);
  define_constant_word ("PAD", O (scratch_area));
  define_constant_word ("#PAD", O (number_scratch_area));
  define_constant_word ("/PAD", BUFFER_SIZE);
  define_constant_word ("PICTURED", O (pictured_string));
  define_constant_word ("#PICTURED", O (number_pictured_string));
  define_constant_word ("/PICTURED", BUFFER_SIZE);
  define_constant_word ("HERE", MEMBER_OFFSET (System, data_pointer));
  define_constant_word ("TP", MEMBER_OFFSET (System, static_pointer));
  define_constant_word ("IS", MEMBER_OFFSET (System, instruction_slot));
  define_constant_word ("IW", MEMBER_OFFSET (System, instruction_word));
  define_constant_word ("ENTRY-SIZE", ENTRY_SIZE);
  define_constant_word ("CELL-SIZE", sizeof (Cell));
  define_constant_word ("CHAR-SIZE", sizeof (Character));
  define_constant_word ("LMB", (Cell) ((((Unsigned_Cell) 1)
					<< (sizeof (Cell) * 8 - 1))));
}

static void
define_bootstrap ()
{
  Cell label1, label2, patch1, patch2;

  define ("COUNT");
  emit_slot (OP_DUP);
  emit_slot_with_instruction (OP_LIT, sizeof (Character));
  emit_slot (OP_PLUS);
  emit_slot (OP_SWAP);
  emit_slot (OP_C_FETCH);
  emit_slot (OP_RET);

  define ("BOOTSTRAP");
  emit_slot (OP_CLEAR_PARAMETER_STACK);
  emit_slot (OP_CLEAR_RETURN_STACK);
  fill_instruction (OP_NOP);
  label1 = sys.data_pointer;
  emit_slot_with_instruction (OP_LIT, 0);
  emit_slot_with_instruction (OP_LIT, O (in));
  emit_slot (OP_STORE);
  emit_slot_with_instruction (OP_LIT, O (input_buffer));
  emit_slot (OP_FETCH);
  emit_slot_with_instruction (OP_LIT, 128);
  emit_call ("ACCEPT-INPUT");
  emit_slot_with_instruction (OP_LIT, 10);
  emit_call ("EMIT");
  emit_slot_with_instruction (OP_LIT, O (number_input_buffer));
  emit_slot (OP_STORE);
  fill_instruction (OP_NOP);
  label2 = sys.data_pointer;
  emit_slot_with_instruction (OP_LIT, ' ');
  emit_call ("SKIP-DELIMITERS");
  emit_slot_with_instruction (OP_LIT, ' ');
  emit_call ("FIND-DELIMITER");
  emit_slot (OP_DUP);
  patch1 = emit_slot_with_instruction (OP_JZ, -1);
  emit_call ("FIND-WORD");
  emit_slot (OP_DUP);
  emit_slot (OP_DUP);
  patch2 = emit_slot_with_instruction (OP_JZ, -1);
  emit_slot (OP_DROP);
  emit_slot_with_instruction (OP_LIT, MEMBER_OFFSET (Entry, code_pointer));
  emit_slot (OP_PLUS);
  emit_slot (OP_FETCH);
  emit_slot (OP_TO_R);  
  emit_slot (OP_EX);
  fill_instruction (OP_NOP);
  emit_slot_with_instruction (OP_JMP, label2);
  fill_instruction (OP_NOP);
  V (patch2) = V (patch1) = sys.data_pointer;
  emit_slot (OP_DROP);
  emit_slot (OP_DROP);
  emit_slot_with_instruction (OP_JMP, label1);
}

void
bootstrap (void)
{
  reset_system ();
  register_core_macros ();
  register_macro ("POSTPONE,", (Function) postpone, 0);
  register_macro ("NUMBER,", (Function) emit_number, 0);
  register_macro ("ACCEPT-INPUT", (Function) accept_input, 2);
  define_words ();
  define_bootstrap ();
  execute (((Entry *) A (sys.task.vocabulary))->code_pointer);
}
