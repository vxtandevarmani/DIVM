#include "../include/common.h"
#include "../include/vm.h"
#include "../include/instr.h"
#include "../include/debug.h"

int main(int argc, char** argv){

    VM VirtualMachine   = {0};
    OpArray code        = {0};

    init_vm(&VirtualMachine);
    init_instructions(&code);

    Value shellcode = {
        .value_type = TYPE_STR,
        .as.type_str = create_string("\x21\x0c\x8e\xd2\x01\x0e\xa0\xf2\xe1\x83\x1f\xf8\x61\xac\x8e\xd2\x81\x2d\xac\xf2\x81\xee\xcd\xf2\x41\xce\xe5\xf2\xe1\x03\x1f\xf8\x21\xed\x8d\xd2\xc1\x6d\xae\xf2\xe1\x65\xc8\xf2\x21\x8c\xed\xf2\xe1\x83\x1e\xf8\x21\x08\x8e\xd2\x01\x8e\xad\xf2\x21\x6d\xcc\xf2\x21\x8c\xee\xf2\xe1\x03\x1e\xf8\xe1\x65\x8a\xd2\x21\x6f\xae\xf2\x81\xae\xcc\xf2\xa1\xed\xe5\xf2\xe1\x83\x1d\xf8\xe1\xe5\x8d\xd2\x01\xae\xac\xf2\xc1\x0d\xc0\xf2\xe1\x03\x1d\xf8\xe1\xa5\x8e\xd2\x61\x4e\xae\xf2\xe1\x45\xcc\xf2\x21\xcd\xed\xf2\xe1\x83\x1c\xf8\xff\x03\x1c\xf8\x01\x05\x80\xd2\xe1\x63\x21\xcb\xe1\x83\x1b\xf8\x01\x07\x80\xd2\xe1\x63\x21\xcb\xe1\x03\x1b\xf8\xe0\x03\x01\xaa\xe1\x43\x01\xd1\xe2\x03\x1f\xaa\x70\x07\x80\xd2\xe1\x66\x02\xd4", 176, &VirtualMachine.head),
    };
    uint32_t const_idx = add_constant(&code, shellcode);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 13, sizeof(uint8_t)); // popping into register 13 can be any number within bound
    
    write_instruction(&code, OP_EVAL);
    write_operands(&code, 1, sizeof(uint8_t));
    write_operands(&code, 13, sizeof(uint8_t));

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
