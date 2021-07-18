#pragma once

#include <Utils/StepTimer.h>

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <atomic>

namespace buma
{

class WindowBase;
class ApplicationBase;

enum PLATFORM_TYPE
{
      PLATFORM_TYPE_WINDOWS
    , PLATFORM_TYPE_SDL
};

struct PLATFORM_DATA_WINDOWS
{
    const void*     hInstance;
    const void*     hPrevInstance;
    const wchar_t*  lpCmdLine;
    int             nCmdShow;
};

struct PLATFORM_DATA_SDL
{
    int          argc;
    const char** argv;
};

struct PLATFORM_DESC
{
    PLATFORM_TYPE   type;
    const void*     data;
};

struct WINDOW_DESC;

class PlatformBase
{
protected:
    PlatformBase();
    PlatformBase(const PlatformBase&) = delete;
    PlatformBase& operator=(const PlatformBase&) = delete;
    virtual ~PlatformBase();

public:
    static PlatformBase* CreatePlatform(const PLATFORM_DESC& _desc);
    bool                 IsPrepared() const { return is_prepared; }
    virtual int          MainLoop() = 0;
    static void          DestroyPlatform(PlatformBase* _platform);

public:
    const std::vector<std::string>&          GetCommandArguments();
    std::vector<std::string>::const_iterator FindArgument(const char* _find_str);
    bool                                     HasArgument(const char* _find_str);

    template<typename Head, typename... Find>
    std::vector<std::string>::const_iterator FindArgument(Head _find_str, Find... _tail) { return HasArgument(_find_str) ? FindArgument(_find_str) : FindArgument(_tail...); }

    template<typename Head, typename... Find>
    bool                                     HasArgument(Head _find_str, Find... _tail) { return HasArgument(_find_str) || HasArgument(_tail...); }

    virtual const PLATFORM_DESC& GetDesc() const = 0;

    virtual ApplicationBase* CreateApplication(const char* _path) = 0;
    virtual void             DestroyApplication(ApplicationBase* _app) = 0;
    virtual void             AttachApplication(ApplicationBase* _app) = 0; // アタッチされたアプリケーションはDestroyApplicationを行う必要はありません。
    ApplicationBase*         GetAttachedApplication() const { return attached_app; }

    virtual WindowBase*      CreateWindow(const WINDOW_DESC& _desc) = 0;
    virtual void             DestroyWindow(WindowBase* _window) = 0;

    virtual void             RequestExit() = 0;
    bool                     ShouldExit() const { return should_exit; }

    const util::StepTimer&   GetStepTimer() const { return timer; }

protected:
    virtual bool ProcessMessage() = 0;

protected:
    std::unique_ptr<std::vector<std::string>>               cmd_lines;
    util::StepTimer                                         timer;
    bool                                                    is_prepared;
    bool                                                    should_exit;
    ApplicationBase*                                        attached_app;

};


}// namespace buma
