typedef struct app_state
{
    int foo;
} app_state;

extern platform_http_response
AppHandleHttpRequest(app_memory *Memory, memory_arena *TranArena, platform_http_request Request)
{
    platform_http_response Result;
    
    BEGIN_BLOCK("AppHandleHttpRequest");
    {    
        if(StringsAreEqual(String("/echo"), Request.Endpoint))
        {
            Result.Code = 200;
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
            Result.Code = 404;
            Result.Payload = String("Not Found");
        }
    }
    END_BLOCK();
    
    return Result;
}
