#pragma once
#include "common.h"

typedef enum{
    TYPE_NIL,
    TYPE_STR,
    TYPE_INT,
    TYPE_FLT,
    TYPE_ADRS,
    TYPE_CHR,
    TYPE_BOOL,
    TYPE_ERROR,
} ValueType;

typedef struct{                 // struct that will stay in heap
    int32_t size;
    char*   chars;
    bool    isMarked;
} String;

typedef struct{                 // im using dynamically typed shit so ig u can call this the main data type the vm wil be operating on
    ValueType value_type;
    union{
        String*     type_str;
        int64_t     type_int;
        float       type_flt;
        bool        type_bool;
        char        type_chr;
        uint8_t*    type_addrs;
    } as;
} Value;

typedef struct MarkerObjs{
    union{
        String*         type_str;
    } as;
    ValueType           type;
    struct MarkerObjs*  next;
} MarkerObjs;

typedef struct{                 // for constant pool, I need to add heap
    int32_t count;
    int32_t capacity;
    Value*  space;
}ConstsArray;

typedef struct{
    int32_t     count;
    int32_t     capacity;        // max of how many elements the heap can store so if its uint16 then the operands have to be 2 bytes
    uint8_t*    code;
    ConstsArray constants;
}OpArray;

typedef enum{
    OP_SYSCALL,     // this opcode will be decoded into asm opcodes and be fed into a string that will execute via fn pointers
    OP_RESOLVE_SSN, // this will call tartarus gate and push the SSN 
    OP_RESOLVE_JMP, // same except it gets the jmp to the syscalls
    OP_RETURN,
    OP_CALL,
    OP_CONST,
    OP_PRINT,
    OP_NEGATE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_NOT,
    OP_SHR,
    OP_SHL,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_NOT_EQUAL,       // implemented because its faster than running EQUAL & NOT SAME AS OTHER INSTRUCITONS
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_CONCAT,
    OP_STRING_EQUAL,
    OP_STRING_NOT_EQUAL,
    OP_EXIT,
    OP_POP_REGISTER,
    OP_PUSH_REGISTER,
    OP_LOAD_LOCAL,
    OP_SET_LOCAL,
    OP_POPN,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,
    OP_JUMP,
    OP_NOP,
    OP_EVAL,
}Opcodes;

uint32_t    add_constant(OpArray* chunk, Value val);
String*     create_string(const char* string, uint32_t length, MarkerObjs** head);
void*       reallocate(void* pointer, uint32_t oldsize, uint32_t newsize);
void        init_instructions(OpArray* instructionArr);
void        write_instruction(OpArray* instructionArr, uint8_t opcode);
void        free_instructions(OpArray* instructionArr);
void        init_consts(ConstsArray* constArr);
void        write_consts(ConstsArray* constArr, Value val);
void        free_consts(ConstsArray* constArr);
void        write_operands(OpArray* instructionArr, uint32_t operand, size_t operand_size);
void        PushNode(MarkerObjs** head, void* value, ValueType type);

#define INITAL_CHUNK 8

#define GROW_FACTOR 2

#define GROW_ARRAY(type, pointer, oldcnt, newcnt) \
        (type*)reallocate(pointer, sizeof(type)*(oldcnt), sizeof(type)*(newcnt))

#define FREE_ARRAY(pointer)  \
        reallocate(pointer, 0, 0);

#define ALLOCATE(type, count)   \
    (type*)reallocate(NULL, 0, sizeof(type)*(count))
