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
   OP_C_FETCH_R, OP_C_STORE_R,
   OP_PICK, OP_R_PICK, OP_DEPTH, OP_MOD, OP_U_MOD, OP_NEGATE,
   OP_THROW, OP_CATCH, OP_MACRO, _OP_MACRO,
   OP_CLEAR_PARAMETER_STACK, OP_CLEAR_RETURN_STACK, OP_DOT_S
  };
typedef enum _Opcode Opcode;

void execute (Cell);
