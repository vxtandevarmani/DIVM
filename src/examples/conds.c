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
    uint32_t constant = add_constant(&code, val);
    write_instruction(&code, OP_CONST);
    write_operands(&code, constant, sizeof(int32_t));

    write_instruction(&code, OP_NOP);

    val.value_type = TYPE_INT;
    val.as.type_int = 2;
    constant = add_constant(&code, val);
    write_instruction(&code, OP_CALL);
    write_operands(&code, 2, sizeof(int32_t));

    write_instruction(&code, OP_PRINT);
    write_instruction(&code, OP_EXIT);

    val.value_type = TYPE_STR;
    val.as.type_str = create_string("daikirai", 8, &VirtualMachine.head);
    constant = add_constant(&code, val);
    write_instruction(&code, OP_CONST);
    write_operands(&code, constant, sizeof(int32_t));

    val.as.type_str = create_string(" is my cute", 11, &VirtualMachine.head);
    constant = add_constant(&code, val);
    write_instruction(&code, OP_CONST);
    write_operands(&code, constant, sizeof(int32_t));

    write_instruction(&code, OP_CONCAT);

    val.as.type_str = create_string(" patooie", 8, &VirtualMachine.head);
    constant = add_constant(&code, val);
    write_instruction(&code, OP_CONST);
    write_operands(&code, constant, sizeof(int32_t));

    write_instruction(&code, OP_CONCAT);

    val.as.type_str = create_string("daikirai is my cute patooie", 27, &VirtualMachine.head);
    constant = add_constant(&code, val);
    write_instruction(&code, OP_CONST);
    write_operands(&code, constant, sizeof(int32_t));

    write_instruction(&code, OP_STRING_EQUAL);

    Value val2 = {
        .value_type = TYPE_FLT,
        .as.type_flt = 3.1,
    };
    constant = add_constant(&code, val2);
    write_instruction(&code, OP_JUMP_IF_TRUE);
    write_operands(&code, 3, sizeof(int32_t));

    write_instruction(&code, OP_RETURN);

    write_instruction(&code, OP_NOP);

    write_instruction(&code, OP_NOP);

    val2.value_type = TYPE_STR;
    val2.as.type_str = create_string("I the SnickerDoodle has awakened!", 33, &VirtualMachine.head);
    constant = add_constant(&code, val2);
    write_instruction(&code, OP_CONST);
    write_operands(&code, constant, sizeof(int32_t));

    write_instruction(&code, OP_PRINT);

    write_instruction(&code, OP_RETURN);


    puts("================ Starting VM ================");
    interpret(&VirtualMachine, &code);    
    free_vm(&VirtualMachine);
    free_instructions(&code);
    return 0;
}

// clang -Wno-error=gnu-label-as-value -Werror -Wall -pedantic main.c instr.c vm.c gc.c  debug.c -O2 -o VMI && ./VMI
