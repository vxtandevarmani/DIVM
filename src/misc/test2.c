#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

typedef enum{
    TYPE_STR,
    TYPE_INT,
    TYPE_FLT,
    TYPE_ADRS,
    TYPE_CHR,
    TYPE_BOOL,
    TYPE_NIL,
    TYPE_ERROR,
} ValueType;

/**
 * @brief
 * according to chatgpt
 * syscall args should be uint64_t
 * syscall rets should be int64_t
 * 
 */

typedef struct{
    ValueType value_type;
    union{
        char*       type_str;
        int64_t     type_int;
        float       type_flt;
        bool        type_bool;
        char        type_chr;
        uint8_t*    type_addrs;
    } as;
} Value;

uint64_t resolve_type(Value val){
    switch(val.value_type){
        case TYPE_BOOL: return (uint64_t)(int64_t)(val.as.type_bool);   // according to chatgpt youre supposed cast from int64 to uint64 to preserve signed bits
        case TYPE_CHR:  return (uint64_t)(int64_t)(val.as.type_chr);
        case TYPE_ADRS: return (uint64_t)(uintptr_t)(val.as.type_addrs);
        case TYPE_INT:  return (uint64_t)(int64_t)(val.as.type_int);
        case TYPE_STR:  return (uint64_t)(uintptr_t)(val.as.type_str);
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

int main() {
    Value registers[26] = {0};

    uint64_t syscallNum = 0x2000004;
 
    registers[1].value_type = TYPE_STR;
    registers[1].as.type_str = "dikinmout.txt";

    registers[2].value_type = TYPE_INT;
    registers[2].as.type_int = O_RDWR | O_CREAT | O_TRUNC;

    registers[3].value_type = TYPE_INT;
    registers[3].as.type_int = 511;         // make sure this shit aint in octal but in decimal

    int64_t retVals          = syscall_exec(0x2000005, registers[1], registers[2], registers[3], registers[4], registers[5], registers[6]);

    memset(registers, 0, sizeof(registers));
   
    registers[1].value_type = TYPE_INT;
    registers[1].as.type_int = retVals;

    registers[2].value_type = TYPE_STR;
    registers[2].as.type_str = "Hello, World!\nLOLOLOLOLOLOLOLOL";

    registers[3].value_type = TYPE_INT;
    registers[3].as.type_int = strlen(registers[2].as.type_str);

    int64_t retVal = syscall_exec(syscallNum, registers[1], registers[2], registers[3], registers[4], registers[5], registers[6]);
    printf("syscall value of write() is %lld\n", retVal);

    memset(registers, 0, sizeof(registers));

    registers[1].value_type = TYPE_INT;
    registers[1].as.type_int = retVals;

   retVal = syscall_exec(0x2000006, registers[1], registers[2], registers[3], registers[4], registers[5], registers[6]); 


    return 0;
}

/*
int main() {
    uint64_t    SSN     = 4;
    int64_t     ret     = 0;
    int64_t     arg1    = 1;
    const char* arg2    = "Hello, World!\n"; 
    size_t      arg3    = strlen(arg2);
    int64_t     arg4    = 0;
    int64_t     arg5    = 0;
    int64_t     arg6    = 0;

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
        :   [SSN] "r"(SSN), [arg1] "r"(arg1), [arg2] "r"(arg2), [arg3] "r"(arg3), [arg4] "r"(arg4),  [arg5] "r"(arg5), [arg6] "r"(arg6)
        :   "x0", "x1", "x2", "x3", "x4", "x5", "x16"
    );

    printf("output for the write syscall is %lld\n", ret);

    return 0; // will never get here
}}*/
