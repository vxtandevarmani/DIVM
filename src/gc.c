#include "../include/gc.h"

void collect_garbage(VM* vMachine){
    Value* stackTop     = vMachine->stackPointer;
    Value* stackBase    = vMachine->stack; 
    int32_t frames = stackTop - stackBase;
    for(int32_t i = 0; i < frames; i++){
        Value val = stackBase[i];
        if(val.value_type == TYPE_STR){
            val.as.type_str->isMarked = true;
        }
    }
    MarkerObjs** head       = &vMachine->head;
    MarkerObjs* current     = *head;
    MarkerObjs* previous    = NULL;
    while(current){
        switch(current->type){
            case TYPE_STR:{
                if(current->as.type_str->isMarked){
                    current->as.type_str->isMarked = false;
                    previous    = current;
                    current     = current->next;
                }
                else{
                    MarkerObjs* unreached = current;
                    current = current->next;
                    if(previous){
                        previous->next = current;
                    }
                    else{
                        *head = current;
                    }
                    if(unreached->as.type_str){
                        //printf("FREED %s\n", unreached->as.type_str->chars);
                        free(unreached->as.type_str->chars);
                        free(unreached->as.type_str);
                        free(unreached);
                    }
                }
                break;
            }
            default:{
                previous    = current;
                current     = current->next;
                break;
            }
        }
    }
    return;
}
