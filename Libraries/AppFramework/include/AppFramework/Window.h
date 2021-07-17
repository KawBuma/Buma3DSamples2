#pragma once

#include <cstdint>

namespace buma
{

class PlatformBase;

enum WINDOW_FLAG
{
    WINDOW_FLAG_NONE            = 0x0,
    WINDOW_FLAG_RESIZABLE       = 0x1,
    WINDOW_FLAG_ALWAYS_ON_TOP   = 0x2,
};
using WINDOW_FLAGS = uint32_t;

enum WINDOW_STYLE
{
    WINDOW_STYLE_WINDOWED,
    WINDOW_STYLE_BORDERLESS,
};

enum WINDOW_STATE
{
    WINDOW_STATE_RESTORE,
    WINDOW_STATE_MAXIMIZE,
    WINDOW_STATE_MINIMIZE,
};

enum WINDOW_PROCESS_FLAG
{
    WINDOW_PROCESS_FLAG_NONE            = 0x0,
    WINDOW_PROCESS_FLAG_RESTORED        = 0x1,
    WINDOW_PROCESS_FLAG_MINIMIZED       = 0x2,
    WINDOW_PROCESS_FLAG_MAXIMIZED       = 0x4,
    WINDOW_PROCESS_FLAG_STYLE_CHANGED   = 0x8,
    WINDOW_PROCESS_FLAG_ACTIVATED       = 0x10,
    WINDOW_PROCESS_FLAG_DEACTIVATED     = 0x20,
    WINDOW_PROCESS_FLAG_MOVED           = 0x40,
    WINDOW_PROCESS_FLAG_RESIZED         = 0x80,
    WINDOW_PROCESS_FLAG_CLOSE           = 0x100,
};
using WINDOW_PROCESS_FLAGS = uint32_t;

struct WINDOW_DESC
{
    const char*     window_title;
    int32_t         offset_x; // INT_MAX を設定してスクリーンの中心にオフセット可能です
    int32_t         offset_y; // INT_MAX を設定してスクリーンの中心にオフセット可能です
    uint32_t        width;
    uint32_t        height;
    WINDOW_STYLE    style;
    WINDOW_STATE    state;
    WINDOW_FLAGS    flags;
};

class WindowBase
{
protected:
    WindowBase();
    WindowBase(const WindowBase&) = delete;
    WindowBase& operator=(const WindowBase&) = delete;
    virtual ~WindowBase();

    virtual bool Init(PlatformBase& _platform, const WINDOW_DESC& _desc) = 0;
    virtual void Destroy() = 0;

public:
    virtual bool            SetWindowTitle(const char* _window_title) = 0;

    virtual bool            OffsetWindowToCenter() = 0;
    virtual bool            OffsetWindow(int32_t _x, int32_t _y) = 0;
    virtual bool            ResizeWindow(uint32_t _w, uint32_t _h) = 0;

    virtual bool            SetWindowStyle(WINDOW_STYLE _style) = 0;
    virtual bool            SetWindowState(WINDOW_STATE _state) = 0;
    virtual bool            SetWindowActive(bool _active) = 0;
    virtual bool            IsWindowActive() const = 0;

    virtual void            RequestClose() = 0;
    virtual bool            ShouldClose() const = 0;

    virtual const char*     GetWindowedTitle() const = 0;
    virtual void            GetWindowedOffset(int32_t* _x, int32_t* _y) const = 0;
    virtual void            GetWindowedSize(uint32_t* _w, uint32_t* _h) const = 0;
    virtual WINDOW_STYLE    GetWindowStyle() const = 0;
    virtual WINDOW_STATE    GetWindowState() const = 0;
    virtual float           GetAspectRatio() const = 0;

    virtual void*           GetNativeHandle() const = 0;

};


}// namespace buma
