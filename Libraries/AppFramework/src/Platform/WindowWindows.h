#pragma once

#include <AppFramework/Platform.h>
#include <AppFramework/Window.h>

#include <Utils/LazyDelegate.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace buma
{

DEFINE_ENUM_FLAG_OPERATORS(WINDOW_PROCESS_FLAGS);

class PlatformWindows;

class WindowWindows : public WindowBase
{
public:
    WindowWindows(PlatformWindows&      _platform,
                  WNDCLASSEXW&          _wnd_class);
    virtual ~WindowWindows();

    bool Init(PlatformBase& _platform, const WINDOW_DESC& _desc) override;

    bool SetWindowTitle(const char* _window_title) override;

    bool OffsetWindowToCenter() override;
    bool OffsetWindow(int32_t _x, int32_t _y) override;
    bool ResizeWindow(uint32_t _w, uint32_t _h) override;

    bool SetWindowStyle(WINDOW_STYLE _style) override;
    bool SetWindowState(WINDOW_STATE _state) override;
    bool SetWindowActive(bool _active) override;
    bool IsWindowActive() const override;

    bool ProcessMessage() override;
    bool Exit() override;

    void AddResizeEvent           (const EventPtr& _event)     override;
    void RemoveResizeEvent        (const EventPtr& _to_remove) override;
    void AddProcessMessageEvent   (const EventPtr& _event)     override;
    void RemoveProcessMessageEvent(const EventPtr& _to_remove) override;
    void AddProcessWindowEvent    (const EventPtr& _event)     override;
    void RemoveProcessWindowEvent (const EventPtr& _to_remove) override;

    const char*             GetWindowedTitle()                              const override;
    void                    GetWindowedOffset(int32_t* _x, int32_t* _y)     const override { *_x = window_desc.offset_x; *_y = window_desc.offset_y; }
    void                    GetWindowedSize(uint32_t* _w, uint32_t* _h)     const override { *_w = window_desc.width; *_h = window_desc.height; }
    WINDOW_STYLE            GetWindowStyle()                                const override { return window_desc.style; }
    WINDOW_STATE            GetWindowState()                                const override { return window_desc.state; }
    float                   GetAspectRatio()                                const override { return aspect_ratio; }
    WINDOW_PROCESS_FLAGS    GetWindowProcessFlags()                         const override { return window_process_flags; }

private:
    bool         CreateWnd(uint32_t _width, uint32_t _height);
    bool         OnResize(ResizeEventArgs* _args);
    WINDOW_STATE GetWindowStateFromHwnd();
    WINDOW_STYLE GetWindowStyleFromHwnd();
    void         GetWindowOffsetFromHwnd(int32_t* _client_x, int32_t* _client_y);
    void         GetWindowSizeFromHwnd(uint32_t* _client_w, uint32_t* _client_h);
    void         GetNearestMonitorResolution(uint32_t* _w, uint32_t* _h);

public:
    static LRESULT CALLBACK WndProc(HWND _hwnd, UINT _message, WPARAM _wparam, LPARAM _lparam);

private:
    PlatformWindows&        platform;
    const WNDCLASSEXW&      wnd_class;
    HWND                    hwnd;
    std::string             window_title;
    WINDOW_DESC             window_desc;
    float                   aspect_ratio;
    WINDOW_PROCESS_FLAGS    window_process_flags;
    bool                    is_window_active;
    bool                    should_exit;
    MSG                     msg;
    LazyDelegate<>          delegate_on_resize;
    LazyDelegate<>          delegate_on_process_message;
    LazyDelegate<>          delegate_on_process_window;

};


}// namespace buma
