#include "../include/instr.h"

/*
    dyanmic array to store the errors       (OPTIONAL)
    The stack                               DONE
    dyanmic array to store the opcodes      DONE
    dynamic array to store the constants    NEED TO ADD STRING SUPPORT * 

    TODO list

    1. STRING                               DONE
    2. STRING_OPS                           DONE
    3. JMP                                  DONE
    4. Functions & callstack                DONE
    5. Managing memory so no mem leaks      DONE
        --> An implementation of this could be a linked
            list for String* objects
    5.5 revise the way to read data because i need to shove in 4 bytes of int instead of 1 byte chars and read that
    6. optimization
        --> add register operations
        --> threaded interpreter
    7. DCUMGATE JIT for indirect syscall stubs
        --> Basically use NtContinue to execute syscalls
            but with JOP chains
*/

void PushNode(MarkerObjs** head, void* value, ValueType type){
    MarkerObjs* temp = ALLOCATE(MarkerObjs, 1);
    switch(type){
        case TYPE_STR:{
            temp->as.type_str = value;  // handle multiple types then there are going to be
            break;
        }
        default:{
            break;  // hopefully unreachable;
        }
    }
    temp->next = *head;
    *head = temp;
    return;
}
void* reallocate(void* pointer, uint32_t oldsize, uint32_t newsize){        // maybe use this to expand and shrink the heap
    if(!newsize){ // newsize == 0;
        free(pointer);
        return NULL;
    }
    void* result = realloc(pointer, newsize);
    if(!result){
        exit(1);                // add garbage collection mode to here
    }
    return result;
}
void init_instructions(OpArray* instructionArr){
    instructionArr->count       = 0;
    instructionArr->capacity    = 0;
    instructionArr->code        = NULL;
    init_consts(&instructionArr->constants);
    return;
}
void write_operands(OpArray* instructionArr, uint32_t operand, size_t operand_size){
    for(int32_t i = operand_size - 1; i >= 0; i--){
        write_instruction(instructionArr, (uint8_t)(operand >> (8 * i) & 0xff));
    }
    return;
}
void write_instruction(OpArray* instructionArr, uint8_t byte){
    if(instructionArr->capacity == instructionArr->count){
        uint32_t oldCapacity  = instructionArr->capacity;
        if(instructionArr->capacity < INITAL_CHUNK){
            instructionArr->capacity = INITAL_CHUNK;
        }
        else{
            instructionArr->capacity *= GROW_FACTOR;
        }
        instructionArr->code = GROW_ARRAY(uint8_t, instructionArr->code, oldCapacity, instructionArr->capacity);
    }
    instructionArr->code[instructionArr->count] = byte;
    instructionArr->count += 1;
    return;
}
void free_instructions(OpArray* instructionArr){
    instructionArr->capacity    = 0;
    instructionArr->count       = 0;
    FREE_ARRAY(instructionArr->code);
    free_consts(&instructionArr->constants);
    return;
}
void init_consts(ConstsArray* constArr){
    constArr->count     = 0;
    constArr->capacity  = 0;
    constArr->space     = NULL;
    return;
}
void write_consts(ConstsArray* constArr, Value val){
    if(constArr->capacity == constArr->count){
        uint32_t oldCap = constArr->capacity;
        if(constArr->capacity < INITAL_CHUNK){
            constArr->capacity = INITAL_CHUNK;
        }
        else{
            constArr->capacity *= 2;
        }
            constArr->space = GROW_ARRAY(Value, constArr->space, oldCap, constArr->capacity);
    }
    constArr->space[constArr->count] = val;
    constArr->count += 1;
    return;
}
void free_consts(ConstsArray* constArr){
    constArr->count     = 0;
    constArr->capacity  = 0;
    FREE_ARRAY(constArr->space);
    return;
}
uint32_t add_constant(OpArray* chunk, Value val){
    write_consts(&chunk->constants, val);
    return chunk->constants.count - 1;
}
String* create_string(const char* chars, uint32_t length, MarkerObjs** head){
    char* heapStr = ALLOCATE(char, length + 1);
    memcpy(heapStr, chars, length);
    heapStr[length] = '\x00';
    String* str     = ALLOCATE(String, 1);
    str->chars      = heapStr;
    str->size       = length;
    str->isMarked   = false;
    PushNode(head, str, TYPE_STR);
    return str;
}
