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

#define O(m)								\
  MEMBER_OFFSET (System, task) + MEMBER_OFFSET (Terminal_Task, m)

System sys;
jmp_buf jmpbuf;

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

void
reset_system (void)
{
  sys.task.vocabulary = 0;
  sys.task.base = 10;
  sys.task.in = 0;
  sys.task.state = 1;
  sys.task.input_buffer = O (terminal_input_buffer);
  sys.task.number_input_buffer = 0;
  sys.task.frames = NULL;
  sys.data_pointer = MEMBER_OFFSET (System, dictionary);
  sys.static_pointer = sys.data_pointer + DICTIONARY_SIZE;
  sys.instruction_slot = 0;
  sys.instruction_word = 0;
  sys.last_slot_offset = 0;
  sys.last_instruction_word = 0;
}

Frame *
make_frame (int t, int s, int rt, int rs,
	    int a, int pc)
{
  Frame *object;

  object = (Frame *) malloc (sizeof (Frame));
  if (object != NULL)
    {
      object->t = t, object->s = s;
      object->rt = rt, object->rs = rs;
      object->a = a, object->pc = pc;
    }
  else
    {
      THROW (-3);
    }
  return object;
}

void
unmake_frame (Frame *object)
{
  free ((char *) object);
}

void
push_frame (Frame *object)
{
  object->link = sys.task.frames;
  sys.task.frames = object;
}

Frame *
pop_frame (void)
{
  if (sys.task.frames != NULL)
    {
      Frame *object;

      object = sys.task.frames;
      sys.task.frames = sys.task.frames->link;
      return object;
    }
  else
    {
      return NULL;
    }
}

void
clear_frames (void)
{
  while (sys.task.frames != NULL)
    unmake_frame (pop_frame ());
}

Cell
allocate (Cell size)
{
  Cell data_pointer;

  data_pointer = sys.data_pointer;
  if ((Unsigned_Cell) (data_pointer + size) > sys.static_pointer)
    THROW (-8);
  else
    sys.data_pointer = data_pointer + size;
  return data_pointer;
}

Cell
static_allocate (Cell size)
{
  int static_pointer;

  static_pointer = sys.static_pointer - size;
  if (static_pointer < sys.data_pointer)
    THROW (-8);
  return sys.static_pointer = static_pointer;
}

void
emit_instruction_slot (Cell slot)
{
  Cell offset, instruction_word, *word_pointer;

  if (sys.instruction_slot == sys.instruction_word)
    {
      sys.instruction_word = allocate (sizeof (Cell));
      sys.instruction_slot = sys.instruction_word;
      sys.instruction_word += sizeof (Cell);
    }
  offset = sys.instruction_word - sys.instruction_slot++;
  offset = sizeof (Cell) - offset;
  instruction_word = sys.instruction_word - sizeof (Cell);
  word_pointer = (Cell *) (((char *) &sys) + instruction_word);
  *word_pointer |= MASK_SLOT (slot, offset);
  sys.last_slot_offset = offset;
  sys.last_instruction_word = instruction_word;
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
  patch = sys.data_pointer;
  emit_instruction_word (word);
  return patch;
}

void
fill_instruction_word (Cell slot)
{
  while (sys.instruction_slot != sys.instruction_word)
    emit_instruction_slot (slot);
}

Cell
word_begin (Character *name, Cell size)
{
  Cell offset;
  Entry *e;

  if (size != 0)
    {
      offset = static_allocate (ENTRY_SIZE);
      sys.instruction_slot = sys.instruction_word = sys.data_pointer;
      sys.task.recursive = sys.data_pointer;
      e = (Entry *) A (offset);
      e->flag = 0;
      e->link = 0;
      size = ((size > ENTRY_NAME_SIZE - 1) ? ENTRY_NAME_SIZE - 1 : size);
      e->name[0] = size;
      size *= sizeof (Character);
      e->code_pointer = sys.data_pointer;
      e->parameter_field = 0;
      memcpy ((char *) (e->name + 1), (char *) name, size);
    }
  else
    {
      THROW (-16);
    }
  return offset;
}

void
word_end (Cell entry_offset)
{
  Entry *e;

  e = (Entry *) A (entry_offset);
  e->link = sys.task.vocabulary;
  sys.task.vocabulary = entry_offset;
}

Cell
find_word (Character *name, Cell size)
{
  Cell vocabulary;
  Entry *e;

  vocabulary = sys.task.vocabulary;
  while (vocabulary != 0)
    {
      e = (Entry *) A (vocabulary);
      if (entry_has_name (e, name, size))
        break;
      vocabulary = e->link;
    }
  return vocabulary;
}

void
put_string (FILE *output, Character *string, Cell size)
{
  Cell i;

  for(i = 0; i < size; i++)
    fputc (string[i], output);
}

void
fatal_error (char *message)
{
  fprintf (stderr, "FATAL ERROR: %s\n", message);
  exit (1);
}
