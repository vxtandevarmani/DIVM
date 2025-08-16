#pragma once
#include "common.h"
#include "vm.h"
#include "instr.h"

size_t  disassemmble_ins(OpArray* insArry, size_t offset, VM* vMachine);
void    disassemble_array(OpArray* insArry, const char* name, VM* vMachine);
void    print_ValueStruct(Value* val, VM* vMachine);
void    PrintNodes(MarkerObjs* head);
