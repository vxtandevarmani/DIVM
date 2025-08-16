#include "../include/common.h"
#include "../include/vm.h"
#include "../include/instr.h"
#include "../include/debug.h"

int main(int argc, char** argv){

    VM VirtualMachine   = {0};
    OpArray code        = {0};

    init_vm(&VirtualMachine);
    init_instructions(&code);

    Value SSN = {
        .value_type = TYPE_INT,
        .as.type_int = 0x2000005,       // open()
    };
    uint32_t const_idx = add_constant(&code, SSN);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));

    Value arg1 = {
        .value_type = TYPE_STR,
        .as.type_str = create_string("readme.txt", 10, &VirtualMachine.head),
    };

    const_idx = add_constant(&code, arg1);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));

    Value arg2 = {
        .value_type = TYPE_INT,
        .as.type_int = O_RDWR | O_CREAT | O_TRUNC,
    };
    const_idx = add_constant(&code, arg2);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));

    Value arg3 = {
        .value_type = TYPE_INT,
        .as.type_int = 438,     /// 666 in octal
    };
    const_idx = add_constant(&code, arg3);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 3, sizeof(uint8_t));

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 2, sizeof(uint8_t));

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 1, sizeof(uint8_t));

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 0, sizeof(uint8_t));

    write_instruction(&code, OP_SYSCALL);

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 1, sizeof(uint8_t));

    write_instruction(&code, OP_EVAL);
    write_operands(&code, 0, sizeof(uint8_t));
    write_operands(&code, 10, sizeof(uint8_t));
    write_operands(&code, 1, sizeof(uint8_t));

    SSN.as.type_int = 0x200004; // write()
    const_idx = add_constant(&code, SSN);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));

    arg2.value_type = TYPE_STR;
    arg2.as.type_str = create_string("HALLO I HAVE BEEN EXECUTED BY WRITE()!!\n", 40, &VirtualMachine.head);
    const_idx = add_constant(&code, arg2);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));

    arg3.value_type = TYPE_INT;
    arg3.as.type_int = arg2.as.type_str->size;
    const_idx = add_constant(&code, arg3);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 3, sizeof(uint8_t));

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 2, sizeof(uint8_t));

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 0, sizeof(uint8_t));

    write_instruction(&code, OP_SYSCALL);


    SSN.as.type_int = 0x2000006;    // close
    const_idx = add_constant(&code, SSN);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));
    
    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 0, sizeof(uint8_t));

    write_instruction(&code, OP_EVAL);
    write_operands(&code, 0, sizeof(uint8_t));
    write_operands(&code, 1, sizeof(uint8_t));
    write_operands(&code, 10, sizeof(uint8_t));

    write_instruction(&code, OP_SYSCALL);

    write_instruction(&code, OP_EXIT);
    disassemble_array(&code, "Instruction Parcel", &VirtualMachine);

    puts("================ Starting VM ================");
    interpret(&VirtualMachine, &code);
    free_vm(&VirtualMachine);
    free_instructions(&code);
    return 0;
}

// clang -Wno-error=gnu-label-as-value -Werror -Wall -pedantic main.c instr.c vm.c gc.c functions.c  debug.c -O2 -o VMI && ./VMI
// clang -Wno-error=gnu-label-as-value -Werror -Wall -pedantic main.c instr.c vm.c gc.c functions.c  debug.c -Os -o VMI && ./VMI
