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
#define DICTIONARY_SIZE 128000

#define ADDRESS_OF(o) (((Byte *) &sys) + (o))
#define A(o) ADDRESS_OF (o)
#define VALUE_OF(o) (*(Cell *) A (o))
#define V(o) VALUE_OF (o)
#define C_VALUE_OF(o) (*(Character *) A (o))
#define CV(o) C_VALUE_OF (o)
#define ALIGN(a, b) ((a + b - 1) & (~(b - 1)))
#define ENTRY_SIZE ALIGN (sizeof (Entry), sizeof (Cell))
#define THROW(n) longjmp (jmpbuf, n)
#define CATCH setjmp (jmpbuf)
#define MEMBER_OFFSET(t, m) ((size_t) &((t *)0)->m)
#define EMPTY_SLOT(o) ~((Unsigned_Cell) 0xff << (o * BYTE_BITS))
#define MASK_SLOT(c,o) ((c & (Unsigned_Cell) 0xff) << (o * BYTE_BITS))

typedef int Cell;
typedef unsigned int Unsigned_Cell;
typedef char Character;
typedef unsigned char Unsigned_Character;
typedef char Byte;
typedef unsigned char Unsigned_Byte;
typedef void (*Function)(void);

struct _Frame
{
  Cell t, s, rt, rs, a, pc;
  struct _Frame *link;
};
typedef struct _Frame Frame;

struct _Entry
{
  Character name[ENTRY_NAME_SIZE];
  Cell code_pointer, parameter_field, flag, link;
};
typedef struct _Entry Entry;

struct _Control_Task
{
  Cell t, s, rt, rs, a, i;
  Cell stack[STACK_SIZE];
  Frame *frames;
};
typedef struct _Control_Task Control_Task;

struct _Terminal_Task
{
  Cell t, s, rt, rs, a, i;
  Cell stack[STACK_SIZE];
  Frame *frames;
  Cell vocabulary, base, in, state, recursive;
  Character terminal_input_buffer[BUFFER_SIZE];
  Cell input_buffer;
  Cell number_input_buffer;
  Character pictured_string[BUFFER_SIZE];
  Cell number_pictured_string;
  Character scratch_area[BUFFER_SIZE];
  Cell number_scratch_area;
};
typedef struct _Terminal_Task Terminal_Task;

struct _System
{
  Terminal_Task task;
  Unsigned_Cell data_pointer, static_pointer;
  Cell instruction_slot, instruction_word;
  Byte dictionary[DICTIONARY_SIZE];
};
typedef struct _System System;

extern jmp_buf jmpbuf;
extern System sys;

void reset_system (void);
Frame * make_frame (int, int, int, int, int, int);
void unmake_frame (Frame *);
void push_frame (Frame *);
Frame *pop_frame (void);
void clear_frames (void);
Cell allocate (Cell);
Cell static_allocate (Cell);
void emit_instruction_slot (Cell);
void emit_instruction_word (Cell);
Cell emit_instruction_slot_and_word (Cell, Cell);
void fill_instruction_word (Cell);
Cell word_begin (Character *, Cell);
void word_end (Cell);
Cell find_word (Character *, Cell);
void put_string (FILE *, Character *, Cell);
void fatal_error (char *);
