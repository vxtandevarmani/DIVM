#pragma once
#include "common.h"
#include "vm.h"

void register_mov(VM* vMachine);
void register_shc_exec(VM* vMachine);

extern void(*FnPtrs[])(VM*);
extern const char* fn_names[];
