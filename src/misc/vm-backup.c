#include "../include/vm.h"
#include "../include/instr.h"
#include "../include/debug.h"
#include "../include/gc.h"


void throw_error(VM* vMachine, const char* instruction, const char* reason){
    int32_t loc = (int32_t)(vMachine->instructionPointer - vMachine->CtxSet->code); // maybe add another parameter to subtract so the offset can point to the correct opcode
    printf("\t\\__[%04d\t ERROR WITH INSTRUCTION: %s]\n\t |_REASON: %s\n", loc, instruction, reason);   // improve logging
    clear_stack(vMachine);
    return;
}

void visualize_stack(VM* vMachine){
    for(Value* i = vMachine->stack; i < vMachine->stackPointer; i++){
        printf("[");
        print_ValueStruct(i, vMachine);
        puts("]");
    }
    return;
}

void clear_stack(VM* vMachine){
   vMachine->stackPointer   = vMachine->stack;
   int32_t stackDelta       = (int32_t)(vMachine->stackPointer - vMachine->stack);
   memset(vMachine->stack, 0, stackDelta);
   return;
}

void init_vm(VM* vMachine){
    clear_stack(vMachine);
    vMachine->head = NULL;
    return;
}
void free_vm(VM* vMachine){
    clear_stack(vMachine); 
    collect_garbage(vMachine);
    return;
}

void push(VM* vMachine, Value val){
    (*vMachine->stackPointer) = val;
    vMachine->stackPointer++;
    return;
}

Value peak_stack(VM* vMachine, int32_t distance){
    return vMachine->stackPointer[- 1 - distance];
}

Value pop(VM* vMachine){
    if(vMachine->stackPointer == vMachine->stack){
        Value err = {
            .value_type  = TYPE_ERROR,
            .as.type_chr = '\x00',
        };
        return err;                     // WHAT VAL SHOULD I PUT >.<
    }
    vMachine->stackPointer--;
    return *vMachine->stackPointer;
}

vmResult run(VM* vMachine){
    Value Pop_Storage[REGISTER_AMOUNT] = {0};
    while(true){
        //disassemmble_ins(vMachine->CtxSet,  vMachine->instructionPointer - vMachine->CtxSet->code, vMachine);   // debug can remove
        //visualize_stack(vMachine);                                                                              // also debug so can remove
        //PrintNodes(vMachine->head);
        uint8_t instruction = *vMachine->instructionPointer;    // read an opcode
        vMachine->instructionPointer++;                         // increment the IP to point to the NEXT instruction
        switch(instruction){
            case OP_NIL:{
                Value val = {
                    .value_type = TYPE_NIL,
                    .as.type_chr = '\x00',
                };
                push(vMachine, val);
                break;
            }
            case OP_TRUE:{
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = true,
                };
                push(vMachine, val);
                break;
            }
            case OP_FALSE:{
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = false,
                };
                push(vMachine, val);
                break;
            }
            case OP_RETURN:{
                Value val = pop(vMachine);
                if(val.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_RETURN", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(val.value_type != TYPE_ADRS){
                    throw_error(vMachine, "OP_RETURN", "Invalid value popped!");
                    return VM_NAY;
                }
                vMachine->instructionPointer = val.as.type_addrs;
                break;
            }
            case OP_CALL:{
                vMachine->instructionPointer += 4;
                int32_t jmpOperand  =   vMachine->instructionPointer[-4] << 24  |
                                        vMachine->instructionPointer[-3] << 16  |
                                        vMachine->instructionPointer[-2] << 8   |
                                        vMachine->instructionPointer[-1];
                Value ret_add = {
                    .value_type     = TYPE_ADRS,
                    .as.type_addrs    = vMachine->instructionPointer,
                };
                push(vMachine, ret_add);
                vMachine->instructionPointer += jmpOperand;
                break;
            }
            case OP_CONST:{
                vMachine->instructionPointer += 4;
                int32_t constIndex  =   vMachine->instructionPointer[-4] << 24 |
                                        vMachine->instructionPointer[-3] << 16 |
                                        vMachine->instructionPointer[-2] << 8  |
                                        vMachine->instructionPointer[-1];
                Value val = vMachine->CtxSet->constants.space[constIndex];
                push(vMachine, val);
                break;
            }
            case OP_NEGATE:{
                Value val = pop(vMachine);
                switch(val.value_type){
                    case TYPE_INT:{
                        val.as.type_int = -val.as.type_int;
                        break;
                    }
                    case TYPE_FLT:{
                        val.as.type_flt = -val.as.type_flt;
                        break;
                    }
                    case TYPE_ERROR:{
                        throw_error(vMachine, "OP_NEGATE", "Stack underflow detected!");
                        return VM_NAY;
                    }
                    default:{
                        throw_error(vMachine, "OP_NEGATE", "Invalid operand type!");
                        return VM_NAY;
                    }
                }
                push(vMachine, val);
                break;
            }
            case OP_ADD:{
                Value b = pop(vMachine);                // 272 in crafting interpreters
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_ADD", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                    throw_error(vMachine, "OP_ADD", "Mismatching operand types!");
                    return VM_NAY;
                }
                if(b.value_type == TYPE_INT){
                    a.as.type_int += b.as.type_int;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_FLT){
                    a.as.type_flt += b.as.type_flt;
                    push(vMachine, a);
                    break;
                }
                else{
                   throw_error(vMachine, "OP_ADD", "Invalid operand types!");
                   return VM_NAY;
                }
            }
            case OP_SUB:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_SUB", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                    throw_error(vMachine, "OP_SUB", "Mismatching operand types!");
                    return VM_NAY;
                }
                if(b.value_type == TYPE_INT){
                    a.as.type_int -= b.as.type_int;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_FLT){
                    a.as.type_flt -= b.as.type_flt;
                    push(vMachine, a); 
                    break;
                }
                else{
                    throw_error(vMachine, "OP_SUB", "Invalid operand types!");
                    return VM_NAY;
                }
            }
            case OP_MUL:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_MUL", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                    throw_error(vMachine, "OP_MUL", "Mismatching operand types!");
                    return VM_NAY;
                }
                if(b.value_type == TYPE_INT){
                    a.as.type_int *= b.as.type_int;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_FLT){
                    a.as.type_flt *= b.as.type_flt;
                   push(vMachine, a); 
                   break;
                }
                else{
                    throw_error(vMachine, "OP_MUL", "Invalid operand types!");
                    return VM_NAY;
                }
            } 
            case OP_DIV:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_DIV", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                    throw_error(vMachine, "OP_DIV", "Mismatching operand types!");
                    return VM_NAY;
                }
                if(b.value_type == TYPE_INT){
                    a.as.type_int /= b.as.type_int;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_FLT){
                    a.as.type_flt /= b.as.type_flt;
                   push(vMachine, a); 
                   break;
                }
                else{
                    throw_error(vMachine, "OP_DIV", "Invalid operand types!");
                    return VM_NAY;
                }
            }
            case OP_NOT:{
                Value val = pop(vMachine);
                if(val.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_NOT", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(val.value_type == TYPE_BOOL){
                    val.as.type_bool = !val.as.type_bool;
                    push(vMachine, val);
                    break;
                }
                else if(val.value_type == TYPE_CHR){
                    val.as.type_chr = ~val.as.type_chr;
                    push(vMachine, val);
                    break;
                }
                else if(val.value_type == TYPE_INT){
                    val.as.type_int = ~val.as.type_int;
                    push(vMachine, val);
                    break;
                }
                else{
                    throw_error(vMachine, "OP_NOT", "Invalid operand types!"); 
                    return VM_NAY;
                }
            }
            case OP_AND:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_AND", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                    throw_error(vMachine, "OP_AND", "Mismatching operand types!");
                    return VM_NAY;
                }
                if(b.value_type == TYPE_BOOL){
                    a.as.type_bool &= b.as.type_bool;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_CHR){
                    a.as.type_chr &= b.as.type_chr;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_INT){
                    a.as.type_int &= b.as.type_int;
                    push(vMachine, a);
                    break;
                }
                else{
                    throw_error(vMachine, "OP_AND", "Invalid operand types!");
                    return VM_NAY;
                }
            }
            case OP_OR:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_OR", "Stack underflow detected!");
                    return VM_NAY;
                } 
                if(b.value_type != a.value_type){
                    throw_error(vMachine, "OP_OR", "Mismatching operand types!");
                    return VM_NAY;
                }
                if(b.value_type == TYPE_BOOL){
                    a.as.type_bool |= b.as.type_bool;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_CHR){
                    a.as.type_chr |= b.as.type_chr;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_INT){
                    a.as.type_int |= b.as.type_int;
                    push(vMachine, a);
                    break;
                }
                else{
                    throw_error(vMachine, "OP_OR", "Invalid operand types!");
                    return VM_NAY;
                }
            }
            case OP_XOR:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_XOR", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                    throw_error(vMachine, "OP_XOR", "Mismatching operand types!");
                    return VM_NAY;
                }
                if(b.value_type == TYPE_BOOL){
                    a.as.type_bool ^= b.as.type_bool;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_CHR){
                    a.as.type_chr ^= b.as.type_chr;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_INT){
                    a.as.type_int ^= b.as.type_int;
                    push(vMachine, a);
                    break;
                }
                else{
                    throw_error(vMachine, "OP_XOR", "Invalid operand types!");
                    return VM_NAY;
                }
            }
            case OP_SHL:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_SHL", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                    throw_error(vMachine, "OP_SHL", "Mismatching operand types!");
                    return VM_NAY;
                }
                if(b.value_type == TYPE_CHR){
                    a.as.type_chr <<= b.as.type_chr;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_INT){
                    a.as.type_int <<= b.as.type_int;
                    push(vMachine, a);
                    break;
                }
                else{
                    throw_error(vMachine, "OP_SHL", "Invalid operand types!");
                    return VM_NAY;
                }
            }
            case OP_SHR:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_SHR", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                    throw_error(vMachine, "OP_SHR", "Mismatching operand types!");
                    return VM_NAY;
                }
                if(b.value_type == TYPE_CHR){
                    a.as.type_chr >>= b.as.type_chr;
                    push(vMachine, a);
                    break;
                }
                else if(b.value_type == TYPE_INT){
                    a.as.type_int >>= b.as.type_int;
                    push(vMachine, a);
                    break;
                }
                else{
                    throw_error(vMachine, "OP_SHR", "Invalid operand types!");
                    return VM_NAY;
                }
            }
            case OP_EQUAL:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_EQUAL", "Stack underflow detected!");
                    return VM_NAY;
                }
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = 0,
                };
                if(b.value_type != a.value_type){
                    val.as.type_bool = false;
                    push(vMachine, val);
                    break;
                }
                switch(b.value_type){
                    case TYPE_NIL:{
                        val.as.type_bool = true;
                        push(vMachine, val);
                        break;
                    }
                    case TYPE_CHR:{
                        val.as.type_bool = (b.as.type_chr == a.as.type_chr);
                        push(vMachine, val);
                        break; 
                    }
                    case TYPE_FLT:{
                        val.as.type_bool = (b.as.type_flt == a.as.type_flt);
                        push(vMachine, val);
                        break;  
                    }
                    case TYPE_INT:{
                        val.as.type_bool = (b.as.type_int == a.as.type_int);
                        push(vMachine, val);
                        break; 
                    }
                    default:{
                        throw_error(vMachine, "OP_EQUAL", "Invalid operand types!");
                        return VM_NAY;
                    }
                }
            }
            case OP_NOT_EQUAL:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_NOT_EQUAL", "Stack underflow detected!");
                    return VM_NAY;
                }
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = 0,
                };
                if(b.value_type != a.value_type){
                    val.as.type_bool = true;
                    push(vMachine, val);
                    break;
                }
                switch(b.value_type){
                    case TYPE_NIL:{
                        val.as.type_bool = false;
                        push(vMachine, val);
                        break;
                    }
                    case TYPE_CHR:{
                        val.as.type_bool = (b.as.type_chr != a.as.type_chr);
                        push(vMachine, val);
                        break; 
                    }
                    case TYPE_FLT:{
                        val.as.type_bool = (b.as.type_flt != a.as.type_flt);
                        push(vMachine, val);
                        break;  
                    }
                    case TYPE_INT:{
                        val.as.type_bool = (b.as.type_int != a.as.type_int);
                        push(vMachine, val);
                        break; 
                    }
                    default:{
                        throw_error(vMachine, "OP_NOT_EQUAL", "Invalid operand types!");
                        return VM_NAY;
                    }
                }
            }
            case OP_GREATER:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_GREATER", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                   throw_error(vMachine, "OP_GREATER", "Mismatching operand types!");
                   return VM_NAY;
                }
                if(b.value_type == TYPE_FLT){
                    Value val = {
                        .value_type = TYPE_BOOL,
                        .as.type_bool = (a.as.type_flt > b.as.type_flt),
                    };
                    push(vMachine, val);
                    break;
                }
                else if(b.value_type == TYPE_INT){
                    Value val = {
                        .value_type = TYPE_BOOL,
                        .as.type_bool = (a.as.type_int > b.as.type_int),
                    };
                    push(vMachine, val);
                    break;
                }
                else{
                    throw_error(vMachine, "OP_GREATER", "Invalid operand types!");
                    return VM_NAY;
                }
            }
            case OP_GREATER_EQUAL:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_GREATER_EQUAL", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                   throw_error(vMachine, "OP_GREATER_EQUAL", "Mismatching operand types!");
                   return VM_NAY;
                }
                if(b.value_type == TYPE_FLT){
                    Value val = {
                        .value_type = TYPE_BOOL,
                        .as.type_bool = (a.as.type_flt > b.as.type_flt) || (a.as.type_flt == b.as.type_flt),
                    };
                    push(vMachine, val);
                    break;
                }
                else if(b.value_type == TYPE_INT){
                    Value val = {
                        .value_type = TYPE_BOOL,
                        .as.type_bool = (a.as.type_int > b.as.type_int) || (a.as.type_int == b.as.type_int),
                    };
                    push(vMachine, val);
                    break;
                }
                else{
                    throw_error(vMachine, "OP_GREATER_EQUAL", "Invalid operand types!");
                    return VM_NAY;
                }   
            }
            case OP_LESS:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_LESS", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                   throw_error(vMachine, "OP_LESS", "Mismatching operand types!");
                   return VM_NAY;
                }
                if(b.value_type == TYPE_FLT){
                    Value val = {
                        .value_type = TYPE_BOOL,
                        .as.type_bool = (a.as.type_flt < b.as.type_flt),
                    };
                    push(vMachine, val);
                    break;
                }
                else if(b.value_type == TYPE_INT){
                    Value val = {
                        .value_type = TYPE_BOOL,
                        .as.type_bool = (a.as.type_int < b.as.type_int),
                    };
                    push(vMachine, val);
                    break;
                }
                else{
                    throw_error(vMachine, "OP_LESS", "Invalid operand types!");
                    return VM_NAY;
                }
            }
            case OP_LESS_EQUAL:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_LESS_EQUAL", "Stack underflow detected!");
                    return VM_NAY;
                }
                if(b.value_type != a.value_type){
                   throw_error(vMachine, "OP_LESS_EQUAL", "Mismatching operand types!");
                   return VM_NAY;
                }
                if(b.value_type == TYPE_FLT){
                    Value val = {
                        .value_type = TYPE_BOOL,
                        .as.type_bool = (a.as.type_flt < b.as.type_flt) || (a.as.type_flt == b.as.type_flt),
                    };
                    push(vMachine, val);
                    break;
                }
                else if(b.value_type == TYPE_INT){
                    Value val = {
                        .value_type = TYPE_BOOL,
                        .as.type_bool = (a.as.type_int < b.as.type_int) || (a.as.type_int == b.as.type_int),
                    };
                    push(vMachine, val);
                    break;
                }
                else{
                    throw_error(vMachine, "OP_LESS_EQUAL", "Invalid operand types!");
                    return VM_NAY;
                }
            }
            case OP_CONCAT:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_CONCAT", "Stack underflow detected!");
                    return VM_NAY;
                }
                size_t bSize    = b.as.type_str->size;
                size_t aSize    = a.as.type_str->size;
                char* newStr    = ALLOCATE(char, bSize + aSize + 1);
                memcpy(newStr, a.as.type_str->chars, aSize);
                memcpy(newStr + aSize, b.as.type_str->chars, bSize);
                newStr[bSize + aSize] = '\x00';

                String* newValStr= ALLOCATE(String, 1);
                newValStr->chars = newStr;
                newValStr->size  = bSize + aSize;
                Value val = {
                    .value_type     = TYPE_STR,
                    .as.type_str    = newValStr,
                };
                push(vMachine, val);
                break;
            }
            case OP_STRING_EQUAL:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_CONCAT", "Stack underflow detected!");
                    return VM_NAY;
                }
                Value val = {
                    .value_type = TYPE_BOOL,
                };
                int32_t bSize    = b.as.type_str->size;
                int32_t aSize    = a.as.type_str->size;
                if(bSize != aSize){
                    val.as.type_bool = false;
                    push(vMachine, val);
                    break;
                }
                val.as.type_bool = (strncmp(b.as.type_str->chars, a.as.type_str->chars, bSize) == 0);
                push(vMachine, val);
                break;
            }
            case OP_STRING_NOT_EQUAL:{
                Value b = pop(vMachine);
                Value a = pop(vMachine);
                if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_CONCAT", "Stack underflow detected!");
                    return VM_NAY;
                }
                Value val = {
                    .value_type     = TYPE_BOOL,
                    .as.type_bool   = 0,
                };
                int32_t bSize    = b.as.type_str->size;
                int32_t aSize    = a.as.type_str->size;
                if(bSize != aSize){
                    val.as.type_bool = true;
                    push(vMachine, val);
                    break;
                }
                val.as.type_bool = (strncmp(b.as.type_str->chars, a.as.type_str->chars, bSize) != 0);
                push(vMachine, val);
                break; 
            }
            case OP_EXIT:{
                return VM_YAY;
            }
            case OP_NOP:{
                break;
            }
            case OP_PRINT:{
                Value val = pop(vMachine);
                if(val.value_type == TYPE_ERROR){
                    throw_error(vMachine, "OP_PRINT", "Stack underflow detected!");
                    return VM_NAY;
                }
                printf("Top value printed: ");
                print_ValueStruct(&val, vMachine);
                puts("");
                break;
            }
            case OP_POP_REGISTER:{
                uint8_t reg_idx = *vMachine->instructionPointer;
                vMachine->instructionPointer++;
                if(reg_idx > REGISTER_AMOUNT){
                    throw_error(vMachine, "OP_POP", "Invalid register number!");
                    return VM_NAY;
                }
                Pop_Storage[reg_idx] = pop(vMachine);
                break;
            }
            case OP_PUSH_REGISTER:{
                uint8_t reg_idx = *vMachine->instructionPointer;
                vMachine->instructionPointer++;
                if(reg_idx > REGISTER_AMOUNT){
                    throw_error(vMachine, "OP_PUSH", "Invalid register number!");
                    return VM_NAY;
                }
                push(vMachine, Pop_Storage[reg_idx]);
                break;
            }
            case OP_LOAD_LOCAL:{
                // Absolutely no idea on this pretend I dont exist for now
                uint32_t offset = *vMachine->instructionPointer;
                vMachine->instructionPointer++;
                push(vMachine, vMachine->stack[offset]);
                break;
            }
            case OP_SET_LOCAL:{
                // Absolutely no idea on this pretend I dont exist for now
                uint32_t offset = *vMachine->instructionPointer;
                vMachine->instructionPointer++;
                vMachine->stack[offset] = peak_stack(vMachine, 0);
                break;
            }
            case OP_JUMP_IF_FALSE:{
                vMachine->instructionPointer += 4;
                int32_t jmpVal  =   vMachine->instructionPointer[-4] << 24 |
                                    vMachine->instructionPointer[-3] << 16 |
                                    vMachine->instructionPointer[-2] << 8  |
                                    vMachine->instructionPointer[-1];
                Value val = peak_stack(vMachine, 0);
                if(val.value_type != TYPE_BOOL){
                    throw_error(vMachine, "OP_JUMP_IF_FALSE", "Top of stack does not represent boolean!");
                    print_ValueStruct(&val, vMachine);
                    return VM_NAY;
                }
                pop(vMachine); 
                if(val.as.type_bool == false){
                    vMachine->instructionPointer += jmpVal;
                }
                break;
            }
            case OP_JUMP_IF_TRUE:{
                vMachine->instructionPointer += 4;
                int32_t jmpVal  =   vMachine->instructionPointer[-4] << 24 |
                                    vMachine->instructionPointer[-3] << 16 |
                                    vMachine->instructionPointer[-2] << 8  |
                                    vMachine->instructionPointer[-1];
                Value val = peak_stack(vMachine, 0);
                if(val.value_type != TYPE_BOOL){
                    throw_error(vMachine, "OP_JUMP_IF_TRUE", "Top of stack does not represent boolean!");
                    print_ValueStruct(&val, vMachine);
                    return VM_NAY;
                }
                pop(vMachine);
                if(val.as.type_bool == true){
                    vMachine->instructionPointer += jmpVal;
                }
                break;
            } 
            case OP_JUMP:{
                vMachine->instructionPointer += 4;
                int32_t jmpVal  =   vMachine->instructionPointer[-4] << 24 |
                                    vMachine->instructionPointer[-3] << 16 |
                                    vMachine->instructionPointer[-2] << 8  |
                                    vMachine->instructionPointer[-1];
                vMachine->instructionPointer += jmpVal;
                break;
            }
            case OP_POPN:{
                vMachine->instructionPointer += 4;
                uint32_t N  =   vMachine->instructionPointer[-4] << 24 |
                                vMachine->instructionPointer[-3] << 16 |
                                vMachine->instructionPointer[-2] << 8  |
                                vMachine->instructionPointer[-1];
                for(uint32_t i = 0; i < N; i++){
                    pop(vMachine);
                }
                break;
            }
            case OP_REGISTER_MOV:{
                uint8_t registerOperand1 = *vMachine->instructionPointer;
                vMachine->instructionPointer++;

                uint8_t registerOperand2 = *vMachine->instructionPointer;
                vMachine->instructionPointer++;

                Pop_Storage[registerOperand1] = Pop_Storage[registerOperand2];
                break;
            }
        }
    }
}
// as of right now page 332 chapter Strings
vmResult interpret(VM* vMachine, OpArray* instructionParcel){
    vMachine->CtxSet                = instructionParcel;
    vMachine->instructionPointer    = instructionParcel->code;
    return run(vMachine); 
}
