#include "../include/debug.h"
#include "../include/functions.h"

/*  the OP_CONST takes in a param
    that can be 1-2 bytes and this
    param represents the index of the
    constant in heap memory
*/

void print_ValueStruct(Value* val, VM* vMachine){
    switch(val->value_type){  
        case TYPE_FLT:{
            printf("%g", val->as.type_flt);
            break;
        }
        case TYPE_CHR:{
            printf("%c", val->as.type_chr);
            break;
        }
        case TYPE_INT:{
           printf("%lld", val->as.type_int);
           break;
        }
        case TYPE_STR:{
            printf("'%s'",val->as.type_str->chars);
            break;
        }
        case TYPE_BOOL:{
            printf("%s", val->as.type_bool ? "true" : "false");
            break;
        }
        case TYPE_NIL:{
            printf("NIL");
            break;
        }
        case TYPE_ADRS:{
            printf("%d", (uint32_t)(val->as.type_addrs - vMachine->CtxSet->code));
            break;
        }
        default:{
            break;
        }
    }
    return;
}

void PrintNodes(MarkerObjs* head){
    MarkerObjs* current = head;
    while(current){
        switch(current->type){
            case TYPE_STR:{
                if(current->as.type_str){
                    printf("Node %p %s %d\n", (void*)current, current->as.type_str->chars, current->as.type_str->isMarked); 
                }
                else{
                    printf("Node %p VALUE FREED %d\n", (void*)current, current->as.type_str->isMarked);  
                }
                break;
            }
            default:{
                printf("Node %p UNKNOWN %d\n", (void*)current, current->as.type_str->isMarked);  
            }
        }
        current = current->next;
    }
    return;
}

size_t disassemmble_ins(OpArray* insArry, size_t offset, VM* vMachine){
    printf("%04zu\t", offset);
    uint8_t instruction = insArry->code[offset];
    switch (instruction){
        case OP_RETURN:{
            puts("OP_RETURN");
            return offset + 1;
        }
        case OP_CONST:{
            int32_t const_idx   =   *(insArry->code + offset + 1) << 24;   // read the next 4 bytes which should be the operand (MUST FIX)
            const_idx           |=  *(insArry->code + offset + 2) << 16;
            const_idx           |=  *(insArry->code + offset + 3) << 8;
            const_idx           |=  *(insArry->code + offset + 4);

            Value val = insArry->constants.space[const_idx];
            printf("OP_CONST ");
            print_ValueStruct(&val, vMachine);
            puts("");
            return offset + 1 + 4;
        }
        case OP_NEGATE:{
            puts("OP_NEGATE");
            return offset + 1;
        }
        case OP_ADD:{
            puts("OP_ADD");
            return offset + 1;
        }
        case OP_SUB:{
            puts("OP_SUB");
            return offset + 1;
        }
        case OP_MUL:{
            puts("OP_MUL");
            return offset + 1;
        }
        case OP_DIV:{
            puts("OP_DIV");
            return offset + 1;
        }
        case OP_NOT:{
            puts("OP_NOT");
            return offset + 1;
        }
        case OP_AND:{
            puts("OP_AND");
            return offset + 1;
        }
        case OP_OR:{
            puts("OP_OR");
            return offset + 1;
        }
        case OP_XOR:{
            puts("OP_XOR");
            return offset + 1;
        }
        case OP_SHL:{
            puts("OP_SHL");
            return offset + 1;
        }
        case OP_SHR:{
            puts("OP_SHR");
            return offset + 1;
        }
        case OP_EQUAL:{
            puts("OP_EQUAL");
            return offset + 1;
        }
        case OP_NOT_EQUAL:{
            puts("OP_NOT_EQUAL");
            return offset + 1;
        }
        case OP_GREATER:{
            puts("OP_GREATER");
            return offset + 1;
        }
        case OP_GREATER_EQUAL:{
            puts("OP_GREATER_EQUAL");
            return offset + 1;
        }
        case OP_LESS:{
            puts("OP_LESS");
            return offset + 1;
        }
        case OP_LESS_EQUAL:{
            puts("OP_LESS");
            return offset + 1;
        }
        case OP_CONCAT:{
            puts("OP_CONCAT");
            return offset + 1;
        }
        case OP_STRING_EQUAL:{
            puts("OP_STRING_EQUAL");
            return offset + 1;
        }
        case OP_STRING_NOT_EQUAL:{
            puts("OP_STRING_NOT_EQUAL");
            return offset + 1;
        }
        case OP_TRUE:{
            puts("OP_TRUE");
            return offset + 1;
        }
        case OP_FALSE:{
            puts("OP_FALSE");
            return offset + 1;
        }
        case OP_NIL:{
            puts("OP_NIL");
            return offset + 1;
        }
        case OP_PRINT:{
            puts("OP_PRINT");
            return offset + 1;
        }
        case OP_EXIT:{
            puts("OP_EXIT");
            return offset + 1;
        }
        case OP_LOAD_LOCAL:{
            // Absolutely no idea on this pretend I dont exist for now
            int32_t operand   =   *(insArry->code + offset + 1) << 24;   // read the next 4 bytes which should be the operand (MUST FIX)
            operand           |=  *(insArry->code + offset + 2) << 16;
            operand           |=  *(insArry->code + offset + 3) << 8;
            operand           |=  *(insArry->code + offset + 4);
            printf("OP_LOAD_LOCAL %d\n", operand);
            return offset + 1 + 4;
        }
        case OP_SET_LOCAL:{
            // Absolutely no idea on this pretend I dont exist for now
            int32_t operand     =   *(insArry->code + offset + 1) << 24;   // read the next 4 bytes which should be the operand (MUST FIX)
            operand             |=  *(insArry->code + offset + 2) << 16;
            operand             |=  *(insArry->code + offset + 3) << 8;
            operand             |=  *(insArry->code + offset + 4);
            printf("OP_SET_LOCAL %d\n", operand);
            return offset + 1 + 4;
        }
        case OP_JUMP_IF_FALSE:{
            int32_t jmp_operand   =   *(insArry->code + offset + 1) << 24;   // read the next 4 bytes which should be the operand (MUST FIX)
            jmp_operand           |=  *(insArry->code + offset + 2) << 16;
            jmp_operand           |=  *(insArry->code + offset + 3) << 8;
            jmp_operand           |=  *(insArry->code + offset + 4);
            printf("OP_JUMP_IF_FALSE %d\n", jmp_operand);
            return offset + 1 + 4;
        }
        case OP_JUMP_IF_TRUE:{
            int32_t jmp_operand   =   *(insArry->code + offset + 1) << 24;   // read the next 4 bytes which should be the operand (MUST FIX)
            jmp_operand           |=  *(insArry->code + offset + 2) << 16;
            jmp_operand           |=  *(insArry->code + offset + 3) << 8;
            jmp_operand           |=  *(insArry->code + offset + 4);
            printf("OP_JUMP_IF_TRUE %d\n", jmp_operand);
            return offset + 1 + 4;
        }
        case OP_JUMP:{
            int32_t jmp_operand =   *(insArry->code + offset + 1) << 24;   // read the next 4 bytes which should be the operand (MUST FIX)
            jmp_operand         |=  *(insArry->code + offset + 2) << 16;
            jmp_operand         |=  *(insArry->code + offset + 3) << 8;
            jmp_operand         |=  *(insArry->code + offset + 4);
            printf("OP_JUMP %4d\n", jmp_operand);
            return offset + 1 + 4;
        }
        case OP_CALL:{
            int32_t jmp_operand =   *(insArry->code + offset + 1) << 24;   // read the next 4 bytes which should be the operand (MUST FIX)
            jmp_operand         |=  *(insArry->code + offset + 2) << 16;
            jmp_operand         |=  *(insArry->code + offset + 3) << 8;
            jmp_operand         |=  *(insArry->code + offset + 4);
            printf("OP_CALL %d\n", jmp_operand);
            return offset + 1 + 4;
        }
        case OP_POPN:{
            uint32_t N   =  *(insArry->code + offset + 1) << 24;   // read the next 4 bytes which should be the operand (MUST FIX)
            N           |=  *(insArry->code + offset + 2) << 16;
            N           |=  *(insArry->code + offset + 3) << 8;
            N           |=  *(insArry->code + offset + 4);
            printf("OP_POPN %4d\n", N);
            return offset + 1 + 4;
        }
        case OP_NOP:{
            puts("OP_NOP");
            return offset + 1;
        }
        case OP_PUSH_REGISTER:{
            uint8_t operand = *(insArry->code + offset + 1);
            printf("OP_PUSH_REGISTER %d\n", operand);
            return offset + 1 + 1;
        }
        case OP_POP_REGISTER:{
            uint8_t operand = *(insArry->code + offset + 1);
            printf("OP_POP_REGISTER %d\n", operand);
            return offset + 1 + 1; 
        }
        case OP_EVAL:{
            uint8_t operand = *(insArry->code + offset + 1);
            printf("OP_EVAL %s\n", fn_names[operand]);
            return offset + 1 + 1;// + 2;
        }
        case OP_SYSCALL:{
            puts("OP_SYSCALL");
            return offset + 1;
        }
        default:{
            printf("[-] UNKNOWN OPCODE OR POSSIBLE OPERAND: %d\n", instruction);
            return offset + 1;
        }
    }
}
void disassemble_array(OpArray* insArry, const char* name, VM* vMachine){
    printf("================ %s ================\n", name);
    for(uint32_t i = 0; i < insArry->count;){
        i = disassemmble_ins(insArry, i, vMachine); 
    }
}
