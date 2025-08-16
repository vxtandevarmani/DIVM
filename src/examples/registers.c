#include "../include/common.h"
#include "../include/vm.h"
#include "../include/instr.h"
#include "../include/debug.h"


int main(void){

    VM VirtualMachine   = {0};
    OpArray code        = {0};

    init_vm(&VirtualMachine);
    init_instructions(&code);

    Value val = {
        .value_type = TYPE_STR,
        .as.type_str = create_string("Hello World!", 12, &VirtualMachine.head),
    };
    uint32_t const_idx = add_constant(&code, val);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 0, sizeof(uint8_t));

    write_instruction(&code, OP_EVAL);
    write_operands(&code, 0, sizeof(uint8_t));  // fn idx
    write_operands(&code, 1, sizeof(uint8_t));  // operand to fn poitner
    write_operands(&code, 0, sizeof(uint8_t));

    write_instruction(&code, OP_PUSH_REGISTER);
    write_operands(&code, 1, sizeof(uint8_t));
    
    write_instruction(&code, OP_PRINT);
    write_instruction(&code, OP_EXIT);
    disassemble_array(&code, "Instruction Parcel", &VirtualMachine);
    //getchar();

    puts("================ Starting VM ================");
    interpret(&VirtualMachine, &code);    
    free_vm(&VirtualMachine);
    free_instructions(&code);
    return 0;
}

// clang -Wno-error=gnu-label-as-value -Werror -Wall -pedantic main.c instr.c vm.c gc.c  debug.c -O2 -o VMI && ./VMI
