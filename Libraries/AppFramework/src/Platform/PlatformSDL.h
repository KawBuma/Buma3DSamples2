#pragma once

#include <AppFramework/Platform.h>

//#include <SDL.h>
union SDL_Event;
struct SDL_Window;

#include <map>
#include <vector>
#include <functional>

namespace buma
{

class WindowSDL;

class PlatformSDL : public PlatformBase
{
public:
    PlatformSDL(const PLATFORM_DESC& _desc);
    ~PlatformSDL();

private:
    bool Init(const PLATFORM_DESC& _desc);
    bool ParseCommandLines(const PLATFORM_DESC& _desc);
    bool PrepareLog();
    void Term();

public:
    const PLATFORM_DESC& GetDesc() const override;

    ApplicationBase*    CreateApplication(const char* _path) override;
    void                DestroyApplication(ApplicationBase* _app) override;
    void                AttachApplication(ApplicationBase* _app) override;

    WindowBase*         CreateWindow(const WINDOW_DESC& _desc) override;
    void                DestroyWindow(WindowBase* _window) override;

    int                 MainLoop() override;
    void                RequestExit() override;

protected:
    bool ProcessMessage() override;

private:
    void OnSDLDisplayEvent(SDL_Event& e);
    void OnSDLWindowEvent(SDL_Event& e);
    void OnSDLSystemEvent(SDL_Event& e);
    static int SDLCunstomEventFilter(void* _userdata, SDL_Event* _event);
    void DestroyPendings();
    bool WasDestroyed(const ApplicationBase* _app) const;

private:
    PLATFORM_DESC                                           desc;
    PLATFORM_DATA_SDL                                       desc_data;
    std::map<SDL_Window*, WindowSDL*>                       windows;
    std::map<ApplicationBase*/*weakref*/, void*/*module*/>  applications;
    std::vector<std::function<void()>>                      pending_destroy;

};


}// namespace buma
