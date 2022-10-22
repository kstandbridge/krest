// NOTE(kstandbridge): This is a mandatory struct you must define, it needs to include Arena
// which will be initialized by the frame work. Place anything in here you want to keep for
// the scope of the application
typedef struct app_state
{
    memory_arena Arena;
    platform_work_queue *WorkQueue;
} app_state;

extern void
AppHandleCommand(app_memory *Memory, string Command, u32 ArgCount, string *Args)
{
    // NOTE(kstandbridge): This is called once at startup for any command line arguments passed.
    // NOTE(kstandbridge): Additionally this will be called again each time a command is entered into the console.
    
    app_state *AppState = Memory->AppState;
    if(StringsAreEqual(Command, String("exit")))
    {
        LogVerbose("Exit requested");
        // NOTE(kstandbridge): Setting this to false causes the main thread to stop looping which will close the application
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
    // NOTE(kstandbridge): This tells the platform layer to read console input
    // we don't want to block the main thread hence why this requires a platform_work_queue
    Platform.InitConsoleCommandLoop(AppState->WorkQueue);
}

internal void
AppTick(app_memory *Memory, f32 dtForFrame)
{
    // NOTE(kstandbridge): This is the main application loop. Its called per frame in GUI based apps. 
    // Console app are set to 2 frames per second
    
    app_state *AppState = Memory->AppState;
}

extern platform_http_response
AppHandleHttpRequest(app_memory *Memory, memory_arena *TranArena, platform_http_request Request)
{
    // NOTE(kstandbridge): This is called for each incoming http request. The arena is automatically freed
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
