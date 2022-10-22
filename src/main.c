typedef struct app_state
{
    memory_arena Arena;
    platform_work_queue *WorkQueue;
} app_state;

extern void
AppHandleCommand(app_memory *Memory, string Command, u32 ArgCount, string *Args)
{
    app_state *AppState = Memory->AppState;
    if(StringsAreEqual(Command, String("exit")))
    {
        LogVerbose("Exit requested");
        Memory->IsRunning = false;
    }
    else
    {
        LogVerbose("Unknown command %S", Command);
    }
}

internal void
AppInit(app_memory *Memory)
{
    app_state *AppState = Memory->AppState;
    AppState->WorkQueue = Platform.MakeWorkQueue(&AppState->Arena, 4);
    Platform.InitConsoleCommandLoop(AppState->WorkQueue);
}

internal void
AppTick(app_memory *Memory, f32 dtForFrame)
{
    app_state *AppState = Memory->AppState;
}

extern platform_http_response
AppHandleHttpRequest(app_memory *Memory, memory_arena *TranArena, platform_http_request Request)
{
    platform_http_response Result;
    
    if(StringsAreEqual(String("/echo"), Request.Endpoint))
    {
        Result.StatusCode = 200;
        if(Request.Payload.Size == 0)
        {
            Result.Payload = String("Hello, world!");
        }
        else
        {
            Result.Payload = FormatString(TranArena, "%S", Request.Payload);
        }
    }
    else
    {
        Result.StatusCode = 404;
        Result.Payload = String("Not Found");
    }
    
    return Result;
}
