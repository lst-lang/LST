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
#include <setjmp.h>
#include <ctype.h>
#include "execute.h"

#define DEFINEINVOKER(n,l)			\
  static Cell invoke_##n (Callable f)		\
  {						\
    Callable_##n f##n;				\
    f##n = (Callable_##n)f;			\
    return (*f##n) l;				\
  }

typedef Cell (*Callable_0) (void);
typedef Cell (*Callable_1) (Cell);
typedef Cell (*Callable_2) (Cell, Cell);
typedef Cell (*Callable_3) (Cell, Cell, Cell);
typedef Cell (*Callable_4) (Cell, Cell, Cell, Cell);

#define NEED_PARAMETERS(n) if (s - n < -1) THROW (-4)
#define MORE_PARAMETERS(n) if (s + n >= rs) THROW (-3)
#define PUSH_PARAMETER(v) MORE_PARAMETERS (1); stack[++s] = t; t = (v)
#define LOWER_PARAMETER(n) stack[s - n]
#define DROP_PARAMETER s--
#define POP_PARAMETER stack[DROP_PARAMETER]
#define NEED_RETURNS(n) if (rs + n > STACK_SIZE) THROW (-6)
#define MORE_RETURNS(n) if (rs - 1 <= s) THROW (-5)
#define PUSH_RETURN(r) MORE_RETURNS (1); stack[--rs] = rt; rt = (r)
#define LOWER_RETURN(n) stack[rs + n]
#define DROP_RETURN rs++
#define POP_RETURN stack[DROP_RETURN]
#define BINARY(e) NEED_PARAMETERS (2); e; DROP_PARAMETER
#define BINARY_OPERATION(op) BINARY (t = LOWER_PARAMETER (0) op t)
#define BINARY_OPERATION_CAST(op,c)		\
  BINARY (t=(c) LOWER_PARAMETER (0) op (c) t)
#define COMPARE_OPERATION(op) BINARY_OPERATION (op); t = -t
#define COMPARE_OPERATION_CAST(op,c)		\
  BINARY_OPERATION_CAST (op, c); t = -t
#define UNARY(e) NEED_PARAMETERS (1); e
#define UNARY_OPERATION(op) UNARY (t = op t)
#define SWAP(a,b) temp = (a); (a) = (b); (b) = temp
#define TRIPLE(e) Cell _t; NEED_PARAMETERS (3); _t = t; e; t = _t
#define TRIPLE_OPERATION(op)					\
  TRIPLE (op (&LOWER_PARAMETER (1), &LOWER_PARAMETER (0), &_t))

jmp_buf jmpbuf;
Cell *data_pointer, *vocabulary;
Byte dictionary[DICTIONARY_SIZE];
Cell macro_stack[MACRO_STACK_SIZE];
Character *terminal_input_buffer;
Cell *input_buffer, *number_input_buffer, *in;
Cell *instruction_slot, *instruction_word, *state;
Cell *exception_handler;

DEFINEINVOKER (0, ())
DEFINEINVOKER (1, (macro_stack[0]))
DEFINEINVOKER (2, (macro_stack[1], macro_stack[0]))
DEFINEINVOKER (3, (macro_stack[2], macro_stack[1],
		   macro_stack[0]))
DEFINEINVOKER (4, (macro_stack[3], macro_stack[2],
		   macro_stack[1], macro_stack[0]))

static Cell
entry_has_name (Entry *e, Character *name, Cell size)
{
  Cell i;

  if (e->name[0] != size)
    return 0;
  for (i = 0; i < size; i++)
    if (toupper (e->name[i + 1]) != toupper (name[i]))
      return 0;
  return 1;
}

static Cell
_min (Cell a, Cell b)
{
  return (a > b) ? b : a;
}

static Character *
make_name (char *c_string)
{
  static Character name[ENTRY_NAME_SIZE];
  Cell i, length;

  length = _min (strlen (c_string), ENTRY_NAME_SIZE);
  for (i = 0; i < length; i++)
    name[i] = c_string[i];
  return name;
}

static Cell
_find_word (Cell _vocabulary, Character *name, Cell size)
{
  Entry *e;

  while (_vocabulary != DICTIONARY_SIZE)
    {
      e = (Entry *) A (_vocabulary);
      if (entry_has_name (e, name, size))
        break;
      _vocabulary = e->link;
    }
  return (_vocabulary == DICTIONARY_SIZE) ? 0 : _vocabulary;
}

static Cell
_macro (char *name, Callable invokee,
	int arguments_number, int drop_result)
{
  static Callable_Invoker invokers[] =
    {
      invoke_0, invoke_1, invoke_2, invoke_3, invoke_4
    };
  Macro *m;
  Cell link, offset;
  
  if (arguments_number > 4)
      fatal_error ("BAD MACRO");
  link = *vocabulary;
  offset = vocabulary_allocate (sizeof (Macro));
  m = (Macro *) A (offset);
  m->invokee = invokee;
  m->arguments_number = arguments_number;
  m->invoker = invokers[arguments_number];
  fill_instruction_word (OP_NOP);
  define (make_name (name), strlen (name), link);
  emit_instruction_slot (OP_LIT);
  emit_instruction_word (offset);
  emit_instruction_slot (OP_MACRO);
  if (drop_result)
    emit_instruction_slot (OP_DROP);
  emit_instruction_slot (OP_RET);
  return ((Entry *) A (*vocabulary))->code_pointer;
}

static void
rot (Cell *x1, Cell *x2, Cell *x3)
{
  Cell n1, n2, n3;

  n1 = *x1, n2 = *x2, n3 = *x3;
  *x1 = n2, *x2 = n3, *x3 = n1;
}

static void
minus_rot (Cell *x1, Cell *x2, Cell *x3)
{
  Cell n1, n2, n3;

  n1 = *x1, n2 = *x2, n3 = *x3;
  *x1 = n3, *x2 = n1, *x3 = n2;
}

void
fatal_error (char *message)
{
  fprintf (stderr, "FATAL ERROR: %s\n", message);
  exit (1);
}

void
put_string (FILE *output, Character *string, Cell size)
{
  Cell i;

  for (i = 0; i < size; i++)
    fputc (string[i], output);
}

void
reset_system (void)
{
  data_pointer = (Cell *) dictionary;
  vocabulary = data_pointer + 1;
  *data_pointer = ((Byte *) (vocabulary + 1) - dictionary);
  *vocabulary = DICTIONARY_SIZE;
  input_buffer = (Cell *) A (allocate (sizeof (Cell)));
  *input_buffer = allocate (sizeof (Character) * BUFFER_SIZE);
  terminal_input_buffer = (Character *) A (*input_buffer);
  number_input_buffer = (Cell *) A (allocate (sizeof (Cell)));
  in = (Cell *) A (allocate (sizeof (Cell)));
  instruction_slot = (Cell *) A (allocate (sizeof (Cell)));
  instruction_word = (Cell *) A (allocate (sizeof (Cell)));
  state = (Cell *) A (allocate (sizeof (Cell)));
  exception_handler = (Cell *) A (allocate (sizeof (Cell)));
  *instruction_slot = *instruction_word = 0;
  *state = 1;
  *exception_handler = -1;
}

Cell
allocate (Cell size)
{
  Cell _data_pointer;

  _data_pointer = *data_pointer;
  if ((Unsigned_Cell) (_data_pointer + size)
      > *vocabulary)
    THROW (-8);
  else
    *data_pointer = _data_pointer + size;
  return _data_pointer;
}

Cell
vocabulary_allocate (Cell size)
{
  int _vocabulary;

  _vocabulary = *vocabulary - size;
  if (_vocabulary < *data_pointer)
    THROW (-8);
  return *vocabulary = _vocabulary;
}

void
emit_instruction_slot (Cell slot)
{
  Cell offset, *word_pointer;

  if (*instruction_slot == *instruction_word)
    {
      *instruction_word = allocate (sizeof (Cell));
      *instruction_slot = *instruction_word;
      *instruction_word += sizeof (Cell);
    }
  offset = *instruction_word - (*instruction_slot)++;
  offset = sizeof (Cell) - offset;
  word_pointer = (Cell *) (dictionary + *instruction_word
			   - sizeof (Cell));
  *word_pointer |= MASK_SLOT (slot, offset);
}

void
emit_instruction_word (Cell word)
{
  Cell offset;

  offset = allocate (sizeof (Cell));
  V (offset) = word;
}

Cell
emit_instruction_slot_and_word (Cell slot, Cell word)
{
  Cell patch;

  emit_instruction_slot (slot);
  patch = *data_pointer;
  emit_instruction_word (word);
  return patch;
}

void
fill_instruction_word (Cell slot)
{
  while (*instruction_slot != *instruction_word)
    emit_instruction_slot (slot);
}

void
define (Character *name, Cell size, Cell link)
{
  Cell offset;
  Entry *e;

  if (size != 0)
    {
      offset = vocabulary_allocate (sizeof (Entry));
      e = (Entry *) A (offset);
      e->flag = 0;
      e->link = link;
      size = _min (size, ENTRY_NAME_SIZE - 1);
      e->name[0] = size;
      size *= sizeof (Character);
      e->code_pointer = *data_pointer;
      e->parameter = 0;
      memcpy ((char *) (e->name + 1), (char *) name, size);
    }
  else
    {
      THROW (-16);
    }
}

Cell
find_word (Character *name, Cell size)
{
  return _find_word (*vocabulary, name, size);
}

Cell
function (char *name, Callable invokee, int arguments_number)
{
  return _macro (name, invokee, arguments_number, 0);
}

Cell
routine (char *name, Callable invokee, int arguments_number)
{
  return _macro (name, invokee, arguments_number, 1);
}

void
execute (Cell pc)
{
  Cell t, s, rt, rs, a, i;
  Cell stack[STACK_SIZE];
  Cell slot, temp;

  t = s = rt = rs = a = i = slot = temp = -1;
  if ((temp = CATCH) != 0)
    {
      PUSH_PARAMETER (temp);
      pc = ((Entry *) A (*exception_handler))->code_pointer;
    }
 next:
  while (pc != -1)
    {
      i = V (pc);
      pc += sizeof (Cell);
      for (slot = 0; slot < (Cell) sizeof (Cell); slot++)
	switch ((((Unsigned_Cell) i) >> (slot * 8)) & 0xff)
	  {
	  case OP_HALT:
	    return;
	  case OP_NOP:
	    continue;
	  case OP_DUP:
	    NEED_PARAMETERS (1);
	    temp = t;
	  _dup:
	    PUSH_PARAMETER (temp);
	    break;
	  case OP_SWAP:
	    NEED_PARAMETERS (2);
	    SWAP (t, LOWER_PARAMETER (0));
	    break;
	  case OP_DROP:
	    NEED_PARAMETERS(1);
	    t = POP_PARAMETER;
	    break;
	  case OP_OVER:
	    NEED_PARAMETERS(2);
	    temp = LOWER_PARAMETER (0);
	    goto _dup;
	  case OP_ROT:
	    { TRIPLE_OPERATION (rot); }
	    break;
	  case OP_MINUS_ROT:
	    { TRIPLE_OPERATION (minus_rot); }
	    break;
	  case OP_NIP:
	    NEED_PARAMETERS (2);
	    DROP_PARAMETER;
	    break;
	  case OP_TUCK:
	    NEED_PARAMETERS (2);
	    PUSH_PARAMETER (t);
	    LOWER_PARAMETER (0) = LOWER_PARAMETER (1);
	    LOWER_PARAMETER (1) = t;
	    break;
	  case OP_ROT_DROP:
	    NEED_PARAMETERS (3);
	    LOWER_PARAMETER (1) = LOWER_PARAMETER (0);
	    DROP_PARAMETER;
	    break;
	  case OP_ROT_DROP_SWAP:
	    NEED_PARAMETERS (3);
	    LOWER_PARAMETER (1) = t;
	    t = LOWER_PARAMETER (0);
	    DROP_PARAMETER;
	    break;
	  case OP_PLUS:
	    BINARY_OPERATION (+);
	    break;
	  case OP_MINUS:
	    BINARY_OPERATION (-);
	    break;
	  case OP_STAR:
	    BINARY_OPERATION (*);
	    break;
	  case OP_SLASH:
	    BINARY_OPERATION (/);
	    break;
	  case OP_U_PLUS:
	    BINARY_OPERATION_CAST (+, Unsigned_Cell);
	    break;
	  case OP_U_MINUS:
	    BINARY_OPERATION_CAST (-, Unsigned_Cell);
	    break;
	  case OP_U_STAR:
	    BINARY_OPERATION_CAST (*, Unsigned_Cell);
	    break;
	  case OP_U_SLASH:
	    BINARY_OPERATION_CAST (/, Unsigned_Cell);
	    break;
	  case OP_ONE_PLUS:
	    UNARY (t++);
	    break;
	  case OP_ONE_MINUS:
	    UNARY (t--);
	    break;
	  case OP_INVERT:
	    UNARY_OPERATION (~);
	    break;
	  case OP_AND:
	    BINARY_OPERATION (&);
	    break;
	  case OP_OR:
	    BINARY_OPERATION (|);
	    break;
	  case OP_XOR:
	    BINARY_OPERATION (^);
	    break;
	  case OP_TWO_STAR:
	    UNARY (t = t << 1);
	    break;
	  case OP_U_TWO_SLASH:
	    UNARY (t = (Cell) ((Unsigned_Cell) t >> 1));
	    break;
	  case OP_TWO_SLASH:
	    UNARY (t = t >> 1);
	    break;
	  case OP_R_SHIFT:
	    BINARY_OPERATION_CAST (>>, Unsigned_Cell);
	    break;
	  case OP_L_SHIFT:
	    BINARY_OPERATION (<<);
	    break;
	  case OP_TRUE:
	    UNARY (t = -1);
	    break;
	  case OP_FALSE:
	    UNARY (t = 0);
	    break;
	  case OP_ZERO_EQUALS:
	    UNARY (t = -(t == 0));
	    break;
	  case OP_ZERO_LESS:
	    UNARY (t = -(t < 0));
	    break;
	  case OP_U_GREATER_THAN:
	    COMPARE_OPERATION_CAST (>, Unsigned_Cell);
	    break;
	  case OP_U_LESS_THAN:
	    COMPARE_OPERATION_CAST (<, Unsigned_Cell);
	    break;
	  case OP_EQUALS:
	    COMPARE_OPERATION (==);
	    break;
	  case OP_U_GREATER_EQUALS:
	    COMPARE_OPERATION_CAST (>=, Unsigned_Cell);
	    break;
	  case OP_U_LESS_EQUALS:
	    COMPARE_OPERATION_CAST (<=, Unsigned_Cell);
	    break;
	  case OP_NOT_EQUALS:
	    COMPARE_OPERATION (!=);
	    break;
	  case OP_GREATER_THAN:
	    COMPARE_OPERATION (>);
	    break;
	  case OP_LESS_THAN:
	    COMPARE_OPERATION (<);
	    break;
	  case OP_GREATER_EQUALS:
	    COMPARE_OPERATION (>=);
	    break;
	  case OP_LESS_EQUALS:
	    COMPARE_OPERATION (<=);
	    break;
	  case OP_TO_R:
	    PUSH_RETURN (t);
	    t = POP_PARAMETER;
	    break;
	  case OP_R_FROM:
	    temp = rt;
	    rt = POP_RETURN;
	    goto _dup;
	  case OP_R_FETCH:
	    temp = rt;
	    goto _dup;
	  case OP_R_FROM_DROP:
	    rt = POP_RETURN;
	    break;
	  case OP_FETCH:
	    UNARY (t = V (t));
	    break;
	  case OP_STORE:
	    BINARY (V (t) = LOWER_PARAMETER (0));
	    t = POP_PARAMETER;
	    break;
	  case OP_C_FETCH:
	    UNARY (t = *A (t));
	    break;
	  case OP_C_STORE:
	    BINARY (*A (t) = LOWER_PARAMETER (0));
	    t = POP_PARAMETER;
	    break;
	  case OP_LIT:
	    PUSH_PARAMETER (V (pc));
	    pc += sizeof (Cell);
	    break;
	  case OP_JMP:
	    pc = V (pc);
	    goto next;
	  case OP_JZ:
	    if (t)
	      {
		pc += sizeof (Cell);
		continue;
	      }
	    else
	      {
		pc = V (pc);
		goto next;
	      }
	  case OP_DRJNE:
	    if (--rt != 0)
	      {
		pc = V (pc);
		goto next;
	      }
	    else
	      {
		rt = POP_RETURN;
		pc += sizeof (Cell);
		continue;
	      }
	  case OP_CALL:
	    PUSH_RETURN (pc + sizeof (Cell));
	    pc = V (pc);
	    goto next;
	  case OP_RET:
	    pc = rt;
	    rt = POP_RETURN;
	    goto next;
	  case OP_EX:
	    NEED_RETURNS (1);
	    temp = rt;
	    rt = pc;
	    pc = temp;
	    goto next;
	  case OP_A:
	    temp = a;
	    goto _dup;
	  case OP_A_STORE:
	    a = t;
	    t = POP_PARAMETER;
	    break;
	  case OP_FETCH_A:
	    PUSH_PARAMETER (V (a));
	    break;
	  case OP_STORE_A:
	    V (a) = t;
	    t = POP_PARAMETER;
	    break;
	  case OP_FETCH_PLUS:
	    PUSH_PARAMETER (V (a));
	    a += sizeof (Cell);
	    break;
	  case OP_STORE_PLUS:
	    V (a) = t;
	    t = POP_PARAMETER;
	    a += sizeof (Cell);
	    break;
	  case OP_FETCH_R:
	    PUSH_PARAMETER (V (rt));
	    break;
	  case OP_STORE_R:
	    V (rt) = t;
	    t = POP_PARAMETER;
	    rt += sizeof (Cell);
	    break;
	  case OP_C_FETCH_A:
	    PUSH_PARAMETER (CV (a));
	    break;
	  case OP_C_STORE_A:
	    CV (a) = t;
	    t = POP_PARAMETER;
	    break;
	  case OP_C_FETCH_PLUS:
	    PUSH_PARAMETER (CV (a));
	    a++;
	    break;
	  case OP_C_STORE_PLUS:
	    CV (a) = t;
	    t = POP_PARAMETER;
	    a++;
	    break;
	  case OP_C_FETCH_R:
	    PUSH_PARAMETER (CV (rt));
	    break;
	  case OP_C_STORE_R:
	    CV (rt) = t;
	    t = POP_PARAMETER;
	    rt++;
	    break;
	  case OP_SAVE:
	    {
	      Cell *frame;

	      NEED_PARAMETERS (1);
	      frame = (Cell *) A (t);
	      t = POP_PARAMETER;
	      *frame++ = t; *frame++ = s;
	      *frame++ = rt; *frame++ = rs;
	      *frame = pc;
	    }
	    break;
	  case OP_RESTORE:
	    {
	      Cell *frame;

	      NEED_PARAMETERS (1);
	      frame = (Cell *) A (t);
	      t = *frame++; s = *frame++;
	      rt = *frame++; rs = *frame++;
	      pc = *frame;
	    }
	    break;
	  case OP_PICK:
	    NEED_PARAMETERS (1 + t);
	    t = LOWER_PARAMETER (t);
	    break;
	  case OP_R_PICK:
	    NEED_RETURNS (t);
	    t = t ? LOWER_RETURN (t - 1) : rt;
	    break;
	  case OP_DEPTH:
	    PUSH_PARAMETER (s);
	    break;
	  case OP_MOD:
	    BINARY_OPERATION (%);
	    break;
	  case OP_U_MOD:
	    BINARY_OPERATION_CAST (%, Unsigned_Cell);
	    break;
	  case OP_NEGATE:
	    UNARY (t = -t);
	    break;
	  case OP_MACRO:
	    {
	      Macro *m;
	      Cell i, arguments_number;

	      NEED_PARAMETERS (1);
	      m = (Macro *) A (t);
	      arguments_number = m->arguments_number;
	      NEED_PARAMETERS (1 + arguments_number);
	      for (i = 0; i < arguments_number; i++)
		{
		  t = POP_PARAMETER;
		  macro_stack[i] = t;
		}
	      t = (*m->invoker) (m->invokee);
	    }
	    break;
	  case OP_CLEAR_PARAMETER_STACK:
	    t = ~0;
	    s = -1;
	    break;
	  case OP_CLEAR_RETURN_STACK:
	    rt = ~0;
	    rs = STACK_SIZE;
	    break;
	  case OP_DOT_S:
	    {
	      int item;

	      for (item = 1; item <= s; item++)
		printf ("%d ", (int) stack[item]);
	      if (s != -1)
		printf ("%d ", (int) t);
	    }
	    break;
	  default:
	    fatal_error ("BAD OPCODE");
	    return;
	  }
    }
}
