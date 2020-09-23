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
#include <setjmp.h>
#include "system.h"
#include "macro.h"
#include "execute.h"

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

static void
rot (Cell* x1, Cell *x2, Cell *x3)
{
  Cell n1, n2, n3;

  n1 = *x1, n2 = *x2, n3 = *x3;
  *x1 = n2, *x2 = n3, *x3 = n1;
}

static void
minus_rot (Cell* x1, Cell *x2, Cell *x3)
{
  Cell n1, n2, n3;

  n1 = *x1, n2 = *x2, n3 = *x3;
  *x1 = n3, *x2 = n1, *x3 = n2;
}

static void
patch_macro (Cell *oprand)
{
  Macro *object;

  object = find_macro ((Character *) A (*oprand));
  if (object != NULL)
    {
      Cell slot;

      slot = static_allocate (sizeof (Macro *));
      if (slot == 0)
	{
	  THROW (-1);
	}
      else
	{
	  *(Macro **) A (slot) = object;
	  *oprand = slot;
	}
    }
  else
    {
      Character *characters;

      characters = (Character *) oprand;
      put_string(stderr, characters + 1, *characters);
      puts ("?");
      fatal_error ("MACRO NOT FOUND");
    }
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
      Frame *top;
      Character abortx[] = { 'A', 'B', 'O', 'R', 'T', 'X' };

      top = pop_frame ();
      if (top == NULL)
	{
	  t = temp, s = 0;
	  rs = STACK_SIZE;
	  pc = find_word (abortx, 6);
	  pc = ((Entry *) A (pc))->code_pointer;
	}
      else
	{
	  t = top->t, s = top->s;
	  rt = top->rt, rs = top->rs;
	  pc = top->pc;
	  unmake_frame (top);
	}
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
	    clear_frames ();
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
	    temp = t;
	    t = POP_PARAMETER;
	    if (temp)
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
	  case OP_THROW:
	    NEED_PARAMETERS (1);
	    temp = t;
	    t = POP_PARAMETER;
	    if (temp)
	      THROW (temp);
	    break;
	  case OP_CATCH:
	    {
	      Frame *top;

	      top = make_frame (t, s, rt, rs, a, pc);
	      push_frame (top);
	    }
	    pc = t;
	    goto next;
	  case OP_MACRO:
	    patch_macro ((Cell *) A (pc));
	    temp = pc - sizeof (Cell);
	    V (temp) &= EMPTY_SLOT (slot);
	    V (temp) |= MASK_SLOT (_OP_MACRO, slot);
	  case _OP_MACRO:
	    {
	      Cell arguments_number;
	      Macro *object;

	      temp = V (pc);
	      object = *(Macro **) A (temp);
	      arguments_number = object->arguments_number;
	      NEED_PARAMETERS (arguments_number);
	      sys.task.s = 0;
	      while (arguments_number--)
		{
		  sys.task.stack[sys.task.s++] = t;
		  t = POP_PARAMETER;
		}
	      pc += sizeof(Cell);
	      temp = (*object->invoker) (object->invokee);
	    }
	    goto _dup;
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
