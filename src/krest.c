#include "krest.h"

#include "win32_kengine_kernel.c"
#include "win32_kengine_generated.c"
#include "win32_kengine_shared.c"
#include "kengine_telemetry.c"


void __stdcall 
mainCRTStartup()
{
    Kernel32 = FindModuleBase(_ReturnAddress());
    Assert(Kernel32);
    
#if KENGINE_INTERNAL
    void *BaseAddress = (void *)Terabytes(2);
#else
    void *BaseAddress = 0;
#endif
    
    u64 StorageSize = Kilobytes(4096);
    void *Storage = Win32VirtualAlloc(BaseAddress, StorageSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    memory_arena Arena;
    ZeroStruct(Arena);
    InitializeArena(&Arena, StorageSize, Storage);
    
    LogVerbose("Hello, world!");
    
    Win32ExitProcess(0);
    
    InvalidCodePath;
}