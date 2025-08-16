#include "../include/functions.h"

// Can instrument debug if needed

void register_mov(VM* vMachine){
    uint8_t reg_idx1 = *vMachine->instructionPointer;
    vMachine->instructionPointer++;
    uint8_t reg_idx2 = *vMachine->instructionPointer;
    vMachine->instructionPointer++;
    vMachine->Pop_Storage[reg_idx1] = vMachine->Pop_Storage[reg_idx2];
    return;
}
void register_shc_exec(VM* vMachine){
    uint8_t reg_idx = *vMachine->instructionPointer;
    vMachine->instructionPointer++;
    if(vMachine->Pop_Storage[reg_idx].value_type != TYPE_STR){
        return;
    }
    char* address = vMachine->Pop_Storage[reg_idx].as.type_str->chars; 

    size_t pagesize = sysconf(_SC_PAGESIZE);
    void *page_start = (void *)((uintptr_t)address & ~(pagesize - 1));
    if(mprotect(page_start, pagesize, PROT_READ | PROT_EXEC) == -1) {
        return;
    }
    ((int(*)(void))address)();
    return;
}

const char* fn_names[] = {
    "register_mov",
    "register_shc_exec",
};

void(*FnPtrs[])(VM*) = {
    register_mov,
    register_shc_exec,
};
