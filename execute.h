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

#define BYTE_BITS 8
#define STACK_SIZE 64
#define BUFFER_SIZE 128
#define ENTRY_NAME_SIZE 32
#define DICTIONARY_SIZE 64000
#define MACRO_STACK_SIZE 4

#define ADDRESS_OF(o) (dictionary + (o))
#define A(o) ADDRESS_OF (o)
#define VALUE_OF(o) (*(Cell *) A (o))
#define V(o) VALUE_OF (o)
#define C_VALUE_OF(o) (*(Character *) A (o))
#define CV(o) C_VALUE_OF (o)
#define ALIGN(a, b) ((a + b - 1) & (~(b - 1)))
#define THROW(n) longjmp (jmpbuf, n)
#define CATCH setjmp (jmpbuf)
#define EMPTY_SLOT(o) ~((Unsigned_Cell) 0xff << (o * BYTE_BITS))
#define MASK_SLOT(c,o) ((c & (Unsigned_Cell) 0xff) << (o * BYTE_BITS))

typedef int Cell;
typedef unsigned int Unsigned_Cell;
typedef unsigned char Byte;
typedef unsigned char Character;
typedef void (*Callable) (void);
typedef Cell (*Callable_Invoker) (Callable);

struct _Entry
{
  Character name[ENTRY_NAME_SIZE];
  Cell code_pointer, parameter, flag, link;
};
typedef struct _Entry Entry;

struct _Macro
{
  Callable invokee;
  Callable_Invoker invoker;
  Cell arguments_number;
};
typedef struct _Macro Macro;

enum _Opcode
  {
   OP_HALT = 0,
   OP_NOP, OP_DUP, OP_SWAP, OP_DROP, OP_OVER,
   OP_ROT, OP_MINUS_ROT, OP_NIP, OP_TUCK,
   OP_ROT_DROP, OP_ROT_DROP_SWAP,
   OP_PLUS, OP_MINUS, OP_STAR, OP_SLASH,
   OP_U_PLUS, OP_U_MINUS, OP_U_STAR, OP_U_SLASH,
   OP_ONE_PLUS, OP_ONE_MINUS, OP_INVERT, OP_AND, OP_OR, OP_XOR,
   OP_TWO_STAR, OP_U_TWO_SLASH, OP_TWO_SLASH, OP_R_SHIFT, OP_L_SHIFT,
   OP_TRUE, OP_FALSE, OP_ZERO_EQUALS, OP_ZERO_LESS,
   OP_U_GREATER_THAN, OP_U_LESS_THAN, OP_EQUALS,
   OP_U_GREATER_EQUALS, OP_U_LESS_EQUALS, OP_NOT_EQUALS,
   OP_GREATER_THAN, OP_LESS_THAN, OP_GREATER_EQUALS, OP_LESS_EQUALS,
   OP_TO_R, OP_R_FROM, OP_R_FETCH, OP_R_FROM_DROP,
   OP_FETCH, OP_STORE, OP_C_FETCH, OP_C_STORE, OP_LIT,
   OP_JMP, OP_JZ, OP_DRJNE, OP_CALL, OP_RET, OP_EX,
   OP_A, OP_A_STORE, OP_FETCH_A, OP_STORE_A, OP_FETCH_PLUS, OP_STORE_PLUS,
   OP_FETCH_R, OP_STORE_R,
   OP_C_FETCH_A, OP_C_STORE_A, OP_C_FETCH_PLUS, OP_C_STORE_PLUS,
   OP_C_FETCH_R, OP_C_STORE_R, OP_SAVE, OP_RESTORE,
   OP_PICK, OP_R_PICK, OP_DEPTH, OP_MOD, OP_U_MOD, OP_NEGATE,
   OP_MACRO, OP_CLEAR_PARAMETER_STACK, OP_CLEAR_RETURN_STACK, OP_DOT_S
  };
typedef enum _Opcode Opcode;

extern jmp_buf jmpbuf;
extern Cell *data_pointer, *vocabulary;
extern Byte dictionary[];
extern Cell macro_stack[];
extern Character *terminal_input_buffer;
extern Cell *input_buffer, *number_input_buffer, *in;
extern Cell *instruction_slot, *instruction_word, *state;
extern Cell *exception_handler;

void fatal_error (char *);
void put_string (FILE *, Character *, Cell);
void reset_system (void);
Cell allocate (Cell);
Cell vocabulary_allocate (Cell);
void emit_instruction_slot (Cell);
void emit_instruction_word (Cell);
void fill_instruction_word (Cell);
void define (Character *, Cell, Cell);
Cell find_word (Character *, Cell);
Cell function (char *, Callable, int);
Cell routine (char *, Callable, int);
void execute (Cell);
