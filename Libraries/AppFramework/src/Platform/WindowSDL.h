#pragma once

#include <AppFramework/Platform.h>
#include <AppFramework/Window.h>

//#include <SDL.h>
union SDL_Event;
struct SDL_Window;

namespace buma
{

class PlatformSDL;

class WindowSDL : public WindowBase
{
public:
    WindowSDL(PlatformSDL& _platform, const WINDOW_DESC& _desc);
    ~WindowSDL();

protected:
    bool Init(PlatformBase& _platform, const WINDOW_DESC& _desc) override;
    void Destroy() override;

public:
    bool SetWindowTitle(const char* _window_title) override;

    bool OffsetWindowToCenter() override;
    bool OffsetWindow(int32_t _x, int32_t _y) override;
    bool ResizeWindow(uint32_t _w, uint32_t _h) override;

    bool SetWindowStyle(WINDOW_STYLE _style) override;
    bool SetWindowState(WINDOW_STATE _state) override;
    bool SetWindowActive(bool _active) override;
    bool IsWindowActive() const override;

    void RequestClose() override;
    bool ShouldClose() const override;

    const char*             GetWindowedTitle()                              const override { return window_title.c_str(); }
    void                    GetWindowedOffset(int32_t* _x, int32_t* _y)     const override { *_x = window_desc.offset_x; *_y = window_desc.offset_y; }
    void                    GetWindowedSize(uint32_t* _w, uint32_t* _h)     const override { *_w = window_desc.width; *_h = window_desc.height; }
    WINDOW_STYLE            GetWindowStyle()                                const override { return window_desc.style; }
    WINDOW_STATE            GetWindowState()                                const override { return window_desc.state; }
    float                   GetAspectRatio()                                const override { return aspect_ratio; }

    void* GetNativeHandle() const override;

public:
    WINDOW_PROCESS_FLAGS OnSDLWindowEvent(const SDL_Event& _event);
    WINDOW_PROCESS_FLAGS UpdateWindowStyles();

private:
    PlatformSDL&                platform;
    SDL_Window*                 sdlwindow;
    std::string                 window_title;
    WINDOW_DESC                 window_desc;
    float                       aspect_ratio;
    bool                        is_window_active;
    std::atomic_bool            should_close;

};


}// namespace buma
