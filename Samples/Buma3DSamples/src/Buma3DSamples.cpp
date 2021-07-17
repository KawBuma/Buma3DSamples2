#include <AppFramework/Framework.h>

#include <Utils/Compiler.h>

int main(int argc, const char** argv)
{
#ifdef BUMA_DEBUG 
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // BUMA_DEBUG

    buma::PlatformBase* platform;
    {
        buma::PLATFORM_DATA_SDL data{ argc, argv };
        buma::PLATFORM_DESC pd{ buma::PLATFORM_TYPE_SDL, &data };
        platform = buma::PlatformBase::CreatePlatform(pd);
    }
    int code = -1;
    if (platform->IsPrepared())
    {
        if (platform->HasArgument("--app"))
        {
            platform->AttachApplication(platform->CreateApplication((++platform->FindArgument("--app"))->c_str()));
        }
        else
        {
            platform->AttachApplication(platform->CreateApplication("./HelloConstantBuffer/HelloConstantBuffer"));
        }
        code = platform->MainLoop();
    }
    buma::PlatformBase::DestroyPlatform(platform);
    return code;
}
