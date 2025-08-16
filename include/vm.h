#pragma once
#include "common.h"
#include "instr.h"

#define STACK_MAX (1024 * 16) // 1024 bytes ~= 1KB
#define REGISTER_AMOUNT 26

typedef enum {
    VM_YAY,
    VM_NAY,
} vmResult;

typedef struct{
    Value       stack[STACK_MAX];
    Value       Pop_Storage[REGISTER_AMOUNT];
    OpArray*    CtxSet;
    Value*      stackPointer;
    uint8_t*    instructionPointer;
    MarkerObjs* head;
} VM;

void        init_vm(VM* vMachine);
void        free_vm(VM* vMachine);
void        clear_stack(VM* vMachine);
void        push(VM* vMachine, Value val);
Value       pop(VM* vMachine);
Value       peak_stack(VM* vMachine, int32_t distance);
vmResult    interpret(VM* vMachine, OpArray* instructionParcel);
vmResult    run(VM* vMachine);
int64_t     syscall_exec(uint64_t SSN, Value arg1, Value arg2, Value arg3, Value arg4, Value arg5, Value arg6);
uint64_t    resolve_type(Value val);

void        register_mov(VM* vMachine);


// IF you wanna use but i personally dont think these should be used as helper macros

#define BINARY_OPERATION(operator, opcode)                              \
    Value b = pop(vMachine);                                            \
    Value a = pop(vMachine);                                            \
    if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){       \
        throw_error(vMachine, (opcode), "Stack underflow detected!");   \
        return VM_NAY;                                                  \
    }                                                                   \
    if(b.value_type != a.value_type){                                   \
        throw_error(vMachine, (opcode), "Mismatching operand types!");  \
        return VM_NAY;                                                  \
    }                                                                   \
    if(b.value_type == TYPE_BOOL){                                      \
        a.as.type_bool = a.as.type_bool operator b.as.type_bool;        \
        push(vMachine, a);                                              \
        break;                                                          \
    }                                                                   \
    else if(b.value_type == TYPE_CHR){                                  \
        a.as.type_chr  = a.as.type_chr operator b.as.type_chr;          \
        push(vMachine, a);                                              \
        break;                                                          \
    }                                                                   \
    else if(b.value_type == TYPE_INT){                                  \
        a.as.type_int = a.as.type_int operator b.as.type_int;           \
        push(vMachine, a);                                              \
        break;                                                          \
    }                                                                   \
    else{                                                               \
        throw_error(vMachine, (opcode), "Invalid operand types!");      \
        return VM_NAY;                                                  \
    }                                                                   \

#define ARITHMETIC_OPERATION(operation, opcode)                         \
    Value b = pop(vMachine);                                            \
    Value a = pop(vMachine);                                            \
    if(b.value_type == TYPE_ERROR || a.value_type == TYPE_ERROR){       \
        throw_error(vMachine, (opcode), "Stack underflow detected!");   \
        return VM_NAY;                                                  \
    }                                                                   \
    if(b.value_type != a.value_type){                                   \
        throw_error(vMachine, (opcode), "Mismatching operand types!");  \
        return VM_NAY;                                                  \
    }                                                                   \
    if(b.value_type == TYPE_INT){                                       \
        a.as.type_int = a.as.type_int operation b.as.type_int;          \
        push(vMachine, a);                                              \
        break;                                                          \
    }                                                                   \
    else if(b.value_type == TYPE_FLT){                                  \
        a.as.type_flt = a.as.type_flt operation b.as.type_flt;          \
        push(vMachine, a);                                              \
        break;                                                          \
    }                                                                   \
    else{                                                               \
        throw_error(vMachine, (opcode), "Invalid operand types!");      \
        return VM_NAY;                                                  \
    }                                                                   \
