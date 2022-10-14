#if KENGINE_INTERNAL
global platform_api *Platform;
global debug_event_table *GlobalDebugEventTable;
#endif

extern void
ApppUpdateFrame(platform_api *PlatformAPI, render_commands *Commands, memory_arena *Arena, app_input *Input)
{
#if KENGINE_INTERNAL
    Platform = PlatformAPI;
    GlobalDebugEventTable = Platform->DebugEventTable;
#endif
    
}