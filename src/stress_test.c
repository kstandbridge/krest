#include "kengine_platform.h"
#include "kengine_memory.h"
#include "kengine_generated.h"
#include "win32_kengine_types.h"
#include "kengine_string.h"
#include "kengine_intrinsics.h"

#ifndef VERSION
#define VERSION 0
#endif // VERSION

#include "win32_kengine_kernel.c"
#include "win32_kengine_generated.c"
#include "win32_kengine_shared.c"

#include "kengine_telemetry.c"
#include "kengine_random.c"

typedef struct task_with_memory
{
    b32 InUse;
    memory_arena Arena;
    
    temporary_memory MemoryFlush;
    
} task_with_memory;

typedef struct app_state
{
    memory_arena TranArena;
    platform_work_queue WorkQueue;
    task_with_memory Tasks[256];
    
} app_state;

internal task_with_memory *
BeginTaskWithMemory(app_state *AppState)
{
    task_with_memory *Result = 0;
    
    for(u32 Index = 0;
        Index < ArrayCount(AppState->Tasks);
        ++Index)
    {
        task_with_memory *Task = AppState->Tasks + Index;
        if(!Task->InUse)
        {
            Task->InUse = true;
            Task->MemoryFlush = BeginTemporaryMemory(&Task->Arena);
            Result = Task;
            break;
        }
    }
    
    return Result;
}

internal void
EndTaskWithMemory(task_with_memory *Task)
{
    EndTemporaryMemory(Task->MemoryFlush);
    
    CompletePreviousWritesBeforeFutureWrites;
    Task->InUse = false;
}

typedef struct spam_http_work
{
    task_with_memory *Task;
    
} spam_http_work;

internal void
SpamHttpThread(spam_http_work *Work)
{
    task_with_memory *Task = Work->Task;
    
    LARGE_INTEGER LastCounter;
    Win32QueryPerformanceCounter(&LastCounter);
    random_state RandomState;
    RandomState.Value = (u32)LastCounter.QuadPart;
    
    u32 SpamCount = RandomU32(&RandomState) % 1000;
    u16 ThreadId = GetThreadID();
    
    for(u32 SpamIndex = 0;
        SpamIndex < SpamCount;
        ++SpamIndex)
    {
        temporary_memory MemoryFlush = BeginTemporaryMemory(&Task->Arena);
        string Request = FormatString(MemoryFlush.Arena, "ThreadId: %u Message: %u Random: %u", ThreadId, SpamIndex, RandomU32(&RandomState));
        u8 *ResponseBuffer = BeginPushSize(MemoryFlush.Arena);
        umm ResponseMaxSize = (MemoryFlush.Arena->CurrentBlock->Size - MemoryFlush.Arena->CurrentBlock->Used);
        umm ResponseSize = Win32SendInternetRequest(String("http://localhost"), 8090, String("/echo"), "GET", 
                                                    Request, ResponseBuffer, ResponseMaxSize, 0, 0, 0);
        EndPushSize(MemoryFlush.Arena, ResponseSize);
        string Response = String_(ResponseSize, ResponseBuffer);
        if(!StringsAreEqual(Request, Response))
        {
            LogError("'%S' does not match '%S'", Request, Response);
        }
        EndTemporaryMemory(MemoryFlush);
    }
    LogVerbose("Sent %u requests", SpamCount);
    
    EndTaskWithMemory(Work->Task);
}

void __stdcall 
mainCRTStartup()
{
    Kernel32 = FindModuleBase(_ReturnAddress());
    Assert(Kernel32);
    
    Platform.AllocateMemory = Win32AllocateMemory;
    Platform.DeallocateMemory = Win32DeallocateMemory;
    
    GlobalWin32State.MemorySentinel.Prev = &GlobalWin32State.MemorySentinel;
    GlobalWin32State.MemorySentinel.Next = &GlobalWin32State.MemorySentinel;
    
    memory_arena Arena_;
    ZeroStruct(Arena_);
    memory_arena *Arena = &Arena_;
    
    HANDLE ProcessHandle = Win32GetCurrentProcess();
    DWORD_PTR ProcessAffintyMask;
    DWORD_PTR SystemAffinityMask;
    u16 ThreadCount = 16;
    if(Win32GetProcessAffinityMask(ProcessHandle, &ProcessAffintyMask, &SystemAffinityMask))
    {
        for(ThreadCount = 0;
            ProcessAffintyMask;
            ProcessAffintyMask >>= 1)
        {
            if(ProcessAffintyMask & 0x1)
            {
                ++ThreadCount;
            }
        }
        
        ThreadCount *= 2;
    }
    
    app_state AppState_;
    ZeroStruct(AppState_);
    app_state *AppState = &AppState_;
    
    LogVerbose("Making %u threads", ThreadCount);
    Win32MakeQueue(&AppState->WorkQueue, ThreadCount);
    
    for(u32 TaskIndex = 0;
        TaskIndex < ArrayCount(AppState->Tasks);
        ++TaskIndex)
    {
        for(;;)
        {
            task_with_memory *Task = BeginTaskWithMemory(AppState);
            if(Task)
            {
                spam_http_work *Work = PushStruct(&Task->Arena, spam_http_work);
                Work->Task = Task;
                Win32AddWorkEntry(&AppState->WorkQueue, SpamHttpThread, Work);
                break;
            }
            else
            {
                _mm_pause();
            }
        }
    }
    
    LogVerbose("Waiting for threads to complete");
    Win32CompleteAllWork(&AppState->WorkQueue);
    LogVerbose("Spam finished");
    EndTelemetryThread();
    
    Win32ExitProcess(0);
    
    InvalidCodePath;
}