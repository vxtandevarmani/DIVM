#include "../include/vm.h"
#include "../include/instr.h"
#include "../include/debug.h"
#include "../include/gc.h"
#include "../include/functions.h"

extern void(*FnPtrs[])(VM*);

uint64_t resolve_type(Value val){
    switch(val.value_type){
        case TYPE_BOOL: return (uint64_t)(int64_t)(val.as.type_bool);   // according to chatgpt youre supposed cast from int64 to uint64 to preserve signed bits
        case TYPE_CHR:  return (uint64_t)(int64_t)(val.as.type_chr);
        case TYPE_ADRS: return (uint64_t)(uintptr_t)(val.as.type_addrs);
        case TYPE_INT:  return (uint64_t)(int64_t)(val.as.type_int);
        case TYPE_STR:  return (uint64_t)(uintptr_t)(val.as.type_str->chars);
        default:        return (uint64_t)(0);
    }
}
int64_t syscall_exec(uint64_t SSN, Value arg1, Value arg2, Value arg3, Value arg4, Value arg5, Value arg6){
    int64_t ret = 0;
    uint64_t a1 = resolve_type(arg1);
    uint64_t a2 = resolve_type(arg2);
    uint64_t a3 = resolve_type(arg3);
    uint64_t a4 = resolve_type(arg4);
    uint64_t a5 = resolve_type(arg5);
    uint64_t a6 = resolve_type(arg6);

    __asm__ volatile (
        "mov x16, %[SSN]\n"
        "mov x0, %[arg1]\n"
        "mov x1, %[arg2]\n"
        "mov x2, %[arg3]\n"
        "mov x3, %[arg4]\n"
        "mov x4, %[arg5]\n"
        "mov x5, %[arg6]\n"
        "svc #0\n"
        "mov %[ret], x0\n"
        :   [ret] "=r"(ret)
        :   [SSN] "r"(SSN), [arg1] "r"(a1), [arg2] "r"(a2), [arg3] "r"(a3), [arg4] "r"(a4),  [arg5] "r"(a5), [arg6] "r"(a6)
        :   "x0", "x1", "x2", "x3", "x4", "x5", "x16"
    );
    return ret;
}
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
    memset(vMachine->Pop_Storage, 0, sizeof(vMachine->Pop_Storage));
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
    vMachine->stackPointer++;           // point to the memory in stack ABOVE the last element
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
    void* jumpLabels[] = {
        &&OP_SYSCALL,
        &&OP_RESOLVE_SSN,
        &&OP_RESOLVE_JMP,
        &&OP_RETURN,
        &&OP_CALL,
        &&OP_CONST,
        &&OP_PRINT,
        &&OP_NEGATE,
        &&OP_ADD,
        &&OP_SUB,
        &&OP_MUL,
        &&OP_DIV,
        &&OP_AND,
        &&OP_OR,
        &&OP_XOR,
        &&OP_NOT,
        &&OP_SHR,
        &&OP_SHL,
        &&OP_NIL,
        &&OP_TRUE,
        &&OP_FALSE,
        &&OP_EQUAL,
        &&OP_NOT_EQUAL,
        &&OP_GREATER,
        &&OP_GREATER_EQUAL,
        &&OP_LESS,
        &&OP_LESS_EQUAL,
        &&OP_CONCAT,
        &&OP_STRING_EQUAL,
        &&OP_STRING_NOT_EQUAL,
        &&OP_EXIT,
        &&OP_POP_REGISTER,
        &&OP_PUSH_REGISTER,
        &&OP_LOAD_LOCAL,
        &&OP_SET_LOCAL,
        &&OP_POPN,
        &&OP_JUMP_IF_FALSE,
        &&OP_JUMP_IF_TRUE,
        &&OP_JUMP,
        &&OP_NOP,
        &&OP_EVAL,
    };

    #ifdef DEBUG
        #define DISPATCH()  disassemmble_ins(vMachine->CtxSet,  vMachine->instructionPointer - vMachine->CtxSet->code, vMachine);   \
                            visualize_stack(vMachine);                                                                              \
                            goto *jumpLabels[*vMachine->instructionPointer++];
    #else
        #define DISPATCH() goto *jumpLabels[*vMachine->instructionPointer++];
    #endif

    DISPATCH();

    OP_SYSCALL:{
        int64_t retval = syscall_exec((uint64_t)vMachine->Pop_Storage[0].as.type_int, vMachine->Pop_Storage[1], vMachine->Pop_Storage[2], vMachine->Pop_Storage[3], vMachine->Pop_Storage[4], vMachine->Pop_Storage[5], vMachine->Pop_Storage[6]);
        Value val = {
            .value_type = TYPE_INT,
            .as.type_int = retval,
        };
        push(vMachine, val);
        DISPATCH();
    }
    OP_RESOLVE_SSN:{
        DISPATCH();
    }
    OP_RESOLVE_JMP:{
        DISPATCH();
    }
    OP_NIL:{
        Value val = {
            .value_type = TYPE_NIL,
            .as.type_chr = '\x00',
        };
        push(vMachine, val);
        DISPATCH();
    }
    OP_TRUE:{
        Value val = {
            .value_type = TYPE_BOOL,
            .as.type_bool = true,
        };
        push(vMachine, val);
        DISPATCH();
    }
    OP_FALSE:{
        Value val = {
            .value_type = TYPE_BOOL,
            .as.type_bool = false,
        };
        push(vMachine, val);
        DISPATCH();
    }
    OP_RETURN:{
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
        DISPATCH();
    }
    OP_CALL:{
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
        DISPATCH();
    }
    OP_CONST:{
        vMachine->instructionPointer += 4;
        uint32_t constIndex  =  vMachine->instructionPointer[-4] << 24 |
                                vMachine->instructionPointer[-3] << 16 |
                                vMachine->instructionPointer[-2] << 8  |
                                vMachine->instructionPointer[-1];
        Value val = vMachine->CtxSet->constants.space[constIndex];
        push(vMachine, val);
        DISPATCH();
    }
    OP_NEGATE:{
        Value val = pop(vMachine);
        switch(val.value_type){
            case TYPE_INT:{
                val.as.type_int = -val.as.type_int;
                DISPATCH();
            }
            case TYPE_FLT:{
                val.as.type_flt = -val.as.type_flt;
                DISPATCH();
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
        DISPATCH();
    }
    OP_ADD:{
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
        switch(b.value_type){
            case TYPE_INT:{
                a.as.type_int += b.as.type_int;
                push(vMachine, a);
                DISPATCH();
            }
            case TYPE_FLT:{
                a.as.type_flt += b.as.type_flt;
                push(vMachine, a);
                DISPATCH();
            }
            default:{
                throw_error(vMachine, "OP_ADD", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_SUB:{
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
        switch(b.value_type){
            case TYPE_INT:{
                a.as.type_int -= b.as.type_int;
                push(vMachine, a);
                DISPATCH();
            }
            case TYPE_FLT:{
                a.as.type_flt -= b.as.type_flt;
                push(vMachine, a); 
                DISPATCH();
            }
            default:{
                throw_error(vMachine, "OP_SUB", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_MUL:{
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
        switch(b.value_type){
            case TYPE_INT:{
                a.as.type_int *= b.as.type_int;
                push(vMachine, a);
                DISPATCH();
            }
            case TYPE_FLT:{
                a.as.type_flt *= b.as.type_flt;
                push(vMachine, a); 
                DISPATCH();
            }
            default:{
                throw_error(vMachine, "OP_MUL", "Invalid operand types!");
                return VM_NAY;
            }
        }
    } 
    OP_DIV:{
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
        switch(b.value_type){
            case TYPE_INT:{
                a.as.type_int /= b.as.type_int;
                push(vMachine, a);
                DISPATCH();
            }
            case TYPE_FLT:{
                a.as.type_flt /= b.as.type_flt;
                push(vMachine, a); 
                DISPATCH();
            }
            default:{
                throw_error(vMachine, "OP_DIV", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_NOT:{
        Value val = pop(vMachine);
        if(val.value_type == TYPE_ERROR){
            throw_error(vMachine, "OP_NOT", "Stack underflow detected!");
            return VM_NAY;
        }
        switch(val.value_type){
            case TYPE_BOOL:{
                val.as.type_bool = !val.as.type_bool;
                push(vMachine, val);
                DISPATCH();
            }
            case TYPE_CHR:{
                val.as.type_chr = ~val.as.type_chr;
                push(vMachine, val);
                DISPATCH();
            }
            case TYPE_INT:{
                val.as.type_int = ~val.as.type_int;
                push(vMachine, val);
                DISPATCH();
            }
            default:{
                throw_error(vMachine, "OP_NOT", "Invalid operand types!"); 
                return VM_NAY;
            }
        }
    }
    OP_AND:{
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
        switch(b.value_type){
            case TYPE_BOOL:{
                a.as.type_bool &= b.as.type_bool;
                push(vMachine, a);
                DISPATCH();               
            }
            case TYPE_CHR:{
                a.as.type_chr &= b.as.type_chr;
                push(vMachine, a);
                DISPATCH(); 
            }
            case TYPE_INT:{
                a.as.type_int &= b.as.type_int;
                push(vMachine, a);
                DISPATCH();               
            }
            default:{
                throw_error(vMachine, "OP_AND", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_OR:{
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
        switch(b.value_type){
            case TYPE_BOOL:{
                a.as.type_bool |= b.as.type_bool;
                push(vMachine, a);
                DISPATCH();       
            }
            case TYPE_CHR:{
                a.as.type_chr |= b.as.type_chr;
                push(vMachine, a);
                DISPATCH();       
            }
            case TYPE_INT:{
                a.as.type_int |= b.as.type_int;
                push(vMachine, a);
                DISPATCH();
            }
            default:{
                throw_error(vMachine, "OP_OR", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_XOR:{
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
        switch(b.value_type){
            case TYPE_BOOL:{
                a.as.type_bool ^= b.as.type_bool;
                push(vMachine, a);
                DISPATCH();
            }
            case TYPE_CHR:{
                a.as.type_chr ^= b.as.type_chr;
                push(vMachine, a);
                DISPATCH();
            }
            case TYPE_INT:{
                a.as.type_int ^= b.as.type_int;
                push(vMachine, a);
                DISPATCH();
            }
            default:{
                throw_error(vMachine, "OP_XOR", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_SHL:{
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
        switch(b.value_type){
            case TYPE_CHR:{
                a.as.type_chr <<= b.as.type_chr;
                push(vMachine, a);
                DISPATCH();
            }
            case TYPE_INT:{
                a.as.type_int <<= b.as.type_int;
                push(vMachine, a);
                DISPATCH();            
            }
            default:{
                throw_error(vMachine, "OP_SHL", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_SHR:{
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
        switch(b.value_type){
            case TYPE_CHR:{
                a.as.type_chr >>= b.as.type_chr;
                push(vMachine, a);
                DISPATCH();
            }
            case TYPE_INT:{
                a.as.type_int >>= b.as.type_int;
                push(vMachine, a);
                DISPATCH();            
            }
            default:{
                throw_error(vMachine, "OP_SHR", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_EQUAL:{
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
            DISPATCH();
        }
        switch(b.value_type){
            case TYPE_NIL:{
                val.as.type_bool = true;
                push(vMachine, val);
                DISPATCH();
            }
            case TYPE_CHR:{
                val.as.type_bool = (b.as.type_chr == a.as.type_chr);
                push(vMachine, val);
                DISPATCH(); 
            }
            case TYPE_FLT:{
                val.as.type_bool = (b.as.type_flt == a.as.type_flt);
                push(vMachine, val);
                DISPATCH();  
            }
            case TYPE_INT:{
                val.as.type_bool = (b.as.type_int == a.as.type_int);
                push(vMachine, val);
                DISPATCH(); 
            }
            default:{
                throw_error(vMachine, "OP_EQUAL", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_NOT_EQUAL:{
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
            DISPATCH();
        }
        switch(b.value_type){
            case TYPE_NIL:{
                val.as.type_bool = false;
                push(vMachine, val);
                DISPATCH();
            }
            case TYPE_CHR:{
                val.as.type_bool = (b.as.type_chr != a.as.type_chr);
                push(vMachine, val);
                DISPATCH(); 
            }
            case TYPE_FLT:{
                val.as.type_bool = (b.as.type_flt != a.as.type_flt);
                push(vMachine, val);
                DISPATCH();  
            }
            case TYPE_INT:{
                val.as.type_bool = (b.as.type_int != a.as.type_int);
                push(vMachine, val);
                DISPATCH(); 
            }
            default:{
                throw_error(vMachine, "OP_NOT_EQUAL", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_GREATER:{
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
        switch(b.value_type){
            case TYPE_FLT:{
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = (a.as.type_flt > b.as.type_flt),
                };
                push(vMachine, val);
                DISPATCH();
            }
            case TYPE_INT:{
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = (a.as.type_int > b.as.type_int),
                };
                push(vMachine, val);
                DISPATCH();
            }
            default:{    
                throw_error(vMachine, "OP_GREATER", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_GREATER_EQUAL:{
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
        switch(b.value_type){
            case TYPE_FLT:{
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = (a.as.type_flt > b.as.type_flt) || (a.as.type_flt == b.as.type_flt),
                };
                push(vMachine, val);
                DISPATCH();
            }
            case TYPE_INT:{
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = (a.as.type_int > b.as.type_int) || (a.as.type_int == b.as.type_int),
                };
                push(vMachine, val);
                DISPATCH();
            }
            default:{
                throw_error(vMachine, "OP_GREATER_EQUAL", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_LESS:{
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
        switch(b.value_type){
            case TYPE_FLT:{
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = (a.as.type_flt < b.as.type_flt),
                };
                push(vMachine, val);
                DISPATCH();   
            }
            case TYPE_INT:{
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = (a.as.type_int < b.as.type_int),
                };
                push(vMachine, val);
                DISPATCH(); 
            }
            default:{
                throw_error(vMachine, "OP_LESS", "Invalid operand types!");
                return VM_NAY; 
            }
        }
    }
    OP_LESS_EQUAL:{
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
        switch(b.value_type){
            case TYPE_FLT:{
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = (a.as.type_flt < b.as.type_flt) || (a.as.type_flt == b.as.type_flt),
                };
                push(vMachine, val);
                DISPATCH();
            }
            case TYPE_INT:{
                Value val = {
                    .value_type = TYPE_BOOL,
                    .as.type_bool = (a.as.type_int < b.as.type_int) || (a.as.type_int == b.as.type_int),
                };
                push(vMachine, val);
                DISPATCH();
            }
            default:{
                throw_error(vMachine, "OP_LESS_EQUAL", "Invalid operand types!");
                return VM_NAY;
            }
        }
    }
    OP_CONCAT:{
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
        DISPATCH();
    }
    OP_STRING_EQUAL:{
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
            DISPATCH();
        }
        val.as.type_bool = (strncmp(b.as.type_str->chars, a.as.type_str->chars, bSize) == 0);
        push(vMachine, val);
        DISPATCH();
    }
    OP_STRING_NOT_EQUAL:{
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
            DISPATCH();
        }
        val.as.type_bool = (strncmp(b.as.type_str->chars, a.as.type_str->chars, bSize) != 0);
        push(vMachine, val);
        DISPATCH(); 
    }
    OP_EXIT:{
        return VM_YAY;
    }
    OP_NOP:{
        DISPATCH();
    }
    OP_PRINT:{
        Value val = pop(vMachine);
        if(val.value_type == TYPE_ERROR){
            throw_error(vMachine, "OP_PRINT", "Stack underflow detected!");
            return VM_NAY;
        }
        printf("Top value printed: ");
        print_ValueStruct(&val, vMachine);
        puts("");
        DISPATCH();
    }
    OP_POP_REGISTER:{
        uint8_t reg_idx = *vMachine->instructionPointer;
        vMachine->instructionPointer++;
        if(reg_idx > REGISTER_AMOUNT){
            throw_error(vMachine, "OP_POP", "Invalid register number!");
            return VM_NAY;
        }
        vMachine->Pop_Storage[reg_idx] = pop(vMachine);
        DISPATCH();
    }
    OP_PUSH_REGISTER:{
        uint8_t reg_idx = *vMachine->instructionPointer;
        vMachine->instructionPointer++;
        if(reg_idx > REGISTER_AMOUNT){
            throw_error(vMachine, "OP_PUSH", "Invalid register number!");
            return VM_NAY;
        }
        push(vMachine, vMachine->Pop_Storage[reg_idx]);
        DISPATCH();
    }
    OP_LOAD_LOCAL:{
        // Absolutely no idea on this pretend I dont exist for now
        vMachine->instructionPointer += 4;
        int32_t offset  =   vMachine->instructionPointer[-4] << 24 |
                            vMachine->instructionPointer[-3] << 16 |
                            vMachine->instructionPointer[-2] << 8  |
                            vMachine->instructionPointer[-1];
        push(vMachine, vMachine->stack[offset]);
        DISPATCH();
    }
    OP_SET_LOCAL:{
        // Absolutely no idea on this pretend I dont exist for now
        vMachine->instructionPointer += 4;
        int32_t offset  =   vMachine->instructionPointer[-4] << 24 |
                            vMachine->instructionPointer[-3] << 16 |
                            vMachine->instructionPointer[-2] << 8  |
                            vMachine->instructionPointer[-1];
        vMachine->stack[offset] = peak_stack(vMachine, 0);
        DISPATCH();
    }
    OP_JUMP_IF_FALSE:{
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
        DISPATCH();
    }
    OP_JUMP_IF_TRUE:{
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
        DISPATCH();
    }
    OP_JUMP:{
        vMachine->instructionPointer += 4;
        int32_t jmpVal  =   vMachine->instructionPointer[-4] << 24 |
                            vMachine->instructionPointer[-3] << 16 |
                            vMachine->instructionPointer[-2] << 8  |
                            vMachine->instructionPointer[-1];
        vMachine->instructionPointer += jmpVal;
        DISPATCH();
    }
    OP_POPN:{
        vMachine->instructionPointer += 4;
        uint32_t N  =   vMachine->instructionPointer[-4] << 24 |
                        vMachine->instructionPointer[-3] << 16 |
                        vMachine->instructionPointer[-2] << 8  |
                        vMachine->instructionPointer[-1];
        for(uint32_t i = 0; i < N; i++){
            pop(vMachine);
        }
        DISPATCH();
    }
    OP_EVAL:{
        uint8_t FnIdx = *vMachine->instructionPointer;
        vMachine->instructionPointer++;
        FnPtrs[FnIdx](vMachine);
        DISPATCH();
    }
}
// as of right now page 332 chapter Strings
vmResult interpret(VM* vMachine, OpArray* instructionParcel){
    vMachine->CtxSet                = instructionParcel;
    vMachine->instructionPointer    = instructionParcel->code;
    return run(vMachine); 
}
