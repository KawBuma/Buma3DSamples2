#include "./PlatformSDL.h"
#include "./WindowSDL.h"

#include <Utils/Definitions.h>
#define BUMA_MIN_LOGTYPE BUMA_LOGTYPE_ALL
#include <Utils/Logger.h>

#include <SDL.h>

#ifdef _WIN32
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#endif // _WIN32

namespace buma
{

WindowSDL::WindowSDL(PlatformSDL& _platform, const WINDOW_DESC& _desc)
    : WindowBase()
    , platform          { _platform } 
    , sdlwindow         {} 
    , window_title      {} 
    , window_desc       {} 
    , aspect_ratio      {} 
    , is_window_active  {} 
    , should_close      {} 
{
    Init(_platform, _desc);
}

WindowSDL::~WindowSDL()
{
    Destroy();
}

bool WindowSDL::Init(PlatformBase& _platform, const WINDOW_DESC& _desc)
{
    aspect_ratio = static_cast<float>(_desc.width) / static_cast<float>(_desc.height);
    should_close = false;

    window_title             = _desc.window_title;
    window_desc              = _desc;
    window_desc.window_title = window_title.c_str();
    window_desc.offset_x     = window_desc.offset_x == INT_MAX ? SDL_WINDOWPOS_CENTERED : window_desc.offset_x;
    window_desc.offset_y     = window_desc.offset_y == INT_MAX ? SDL_WINDOWPOS_CENTERED : window_desc.offset_y;
    window_desc.width        = std::max(window_desc.width , 128u);
    window_desc.height       = std::max(window_desc.height, 128u);

    Uint32 flags{};
    if (_desc.flags & WINDOW_FLAG_RESIZABLE)     flags |= SDL_WINDOW_RESIZABLE;
    if (_desc.flags & WINDOW_FLAG_ALWAYS_ON_TOP) flags |= SDL_WINDOW_ALWAYS_ON_TOP;

    is_window_active = true;
    switch (_desc.state)
    {
    case buma::WINDOW_STATE_RESTORE:
        break;
    case buma::WINDOW_STATE_MAXIMIZE:
        flags |= SDL_WINDOW_MAXIMIZED;
        break;
    case buma::WINDOW_STATE_MINIMIZE:
        flags |= SDL_WINDOW_MINIMIZED;
        is_window_active = false;
        break;
    default:
        return false;
    }

    switch (_desc.style)
    {
    case buma::WINDOW_STYLE_WINDOWED: break;
    case buma::WINDOW_STYLE_BORDERLESS: flags |= SDL_WINDOW_BORDERLESS; break;
    default:
        return false;
    }

    sdlwindow = SDL_CreateWindow(window_desc.window_title, window_desc.offset_x, window_desc.offset_y, window_desc.width, window_desc.height, flags);
    BUMA_ASSERT(sdlwindow);
    if (sdlwindow == nullptr)
        return false;

    SDL_GetWindowPosition(sdlwindow, &window_desc.offset_x, &window_desc.offset_y);
    SDL_SetWindowMinimumSize(sdlwindow, 128, 128);

    BUMA_LOGI("opened \"{}\"", window_title);
    return true;
}

void WindowSDL::Destroy()
{
    if (sdlwindow)
    {
        SDL_DestroyWindow(sdlwindow);
        sdlwindow = nullptr;
        BUMA_LOGI("closed \"{}\"", window_title);
    }
}

bool WindowSDL::SetWindowTitle(const char* _window_title)
{
    SDL_SetWindowTitle(sdlwindow, _window_title);
    return true;
}

bool WindowSDL::OffsetWindowToCenter()
{
    SDL_SetWindowPosition(sdlwindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    return true;
}

bool WindowSDL::OffsetWindow(int32_t _x, int32_t _y)
{
    if (_x == INT_MAX) _x = SDL_WINDOWPOS_CENTERED;
    if (_y == INT_MAX) _y = SDL_WINDOWPOS_CENTERED;
    SDL_SetWindowPosition(sdlwindow, _x, _y);
    return true;
}

bool WindowSDL::ResizeWindow(uint32_t _w, uint32_t _h)
{
    SDL_SetWindowSize(sdlwindow, static_cast<uint32_t>(_w), static_cast<uint32_t>(_h));
    return true;
}

bool WindowSDL::SetWindowStyle(WINDOW_STYLE _style)
{
    switch (_style)
    {
    case buma::WINDOW_STYLE_WINDOWED:
        SDL_SetWindowBordered(sdlwindow, SDL_TRUE);
        break;
    case buma::WINDOW_STYLE_BORDERLESS:
        SDL_SetWindowBordered(sdlwindow, SDL_FALSE);
        break;
    default:
        return false;
    }
    return true;
}

bool WindowSDL::SetWindowState(WINDOW_STATE _state)
{
    switch (_state)
    {
    case buma::WINDOW_STATE_RESTORE:
        SDL_RestoreWindow(sdlwindow);
        break;
    case buma::WINDOW_STATE_MAXIMIZE:
        SDL_MaximizeWindow(sdlwindow);
        break;
    case buma::WINDOW_STATE_MINIMIZE:
        SDL_MinimizeWindow(sdlwindow);
        break;
    default:
        return false;
    }
    return true;
}

bool WindowSDL::SetWindowActive(bool _active)
{
    if (is_window_active == _active)
        return true;

    if (_active)
    {
        SDL_RaiseWindow(sdlwindow);
    }
    else
    {
        // HACK: 現在のウィンドウの非アクティブ化には「別の」ウィンドウが必要です。
        auto tmp = SDL_CreateWindow(__func__, 0, 0, 1, 1, SDL_WINDOW_HIDDEN | SDL_WINDOW_SKIP_TASKBAR);
        SDL_ShowWindow(tmp);
        SDL_DestroyWindow(tmp);
    }
    return true;
}

bool WindowSDL::IsWindowActive() const
{
    return is_window_active;
}

void WindowSDL::RequestClose()
{

    // SDLイベントプロシージャ外からのウィンドウを閉じる要求を処理することを意図しています。

    should_close = true;
    BUMA_LOGI("close requested \"{}\"", window_title);

    SDL_Event e{ SDL_WINDOWEVENT };
    e.window.type      = SDL_WINDOWEVENT;
    e.window.timestamp = SDL_GetTicks();
    e.window.windowID  = SDL_GetWindowID(sdlwindow);
    e.window.event     = SDL_WINDOWEVENT_CLOSE;
    SDL_PushEvent(&e);
}

bool WindowSDL::ShouldClose() const
{
    return should_close;
}

void* WindowSDL::GetNativeHandle() const
{
    // FIXME: SDLを使用した実装であるため、ネイティブハンドルをSDL_Windowとしています。 
    // SDL_syswm.hをインクルードして、  SDL_GetWindowWMInfo() から実際のネイティブハンドルを取得します。
    return sdlwindow;
}

const char* WE[] = {
    "SDL_WINDOWEVENT_NONE",
    "SDL_WINDOWEVENT_SHOWN",
    "SDL_WINDOWEVENT_HIDDEN",
    "SDL_WINDOWEVENT_EXPOSED",

    "SDL_WINDOWEVENT_MOVED",

    "SDL_WINDOWEVENT_RESIZED",
    "SDL_WINDOWEVENT_SIZE_CHANGED",

    "SDL_WINDOWEVENT_MINIMIZED",
    "SDL_WINDOWEVENT_MAXIMIZED",
    "SDL_WINDOWEVENT_RESTORED",

    "SDL_WINDOWEVENT_ENTER",
    "SDL_WINDOWEVENT_LEAVE",
    "SDL_WINDOWEVENT_FOCUS_GAINED",
    "SDL_WINDOWEVENT_FOCUS_LOST",
    "SDL_WINDOWEVENT_CLOSE",
    "SDL_WINDOWEVENT_TAKE_FOCUS",
    "SDL_WINDOWEVENT_HIT_TEST"
};
WINDOW_PROCESS_FLAGS WindowSDL::OnSDLWindowEvent(const SDL_Event& _event)
{
    WINDOW_PROCESS_FLAGS result = WINDOW_PROCESS_FLAG_NONE;
    auto&& w = _event.window;

    BUMA_LOGT(WE[w.event]);

    switch (w.event)
    {
    case SDL_WINDOWEVENT_SHOWN:
        break;
    case SDL_WINDOWEVENT_HIDDEN:
        break;

    case SDL_WINDOWEVENT_EXPOSED:
        break;

    case SDL_WINDOWEVENT_MOVED:
        window_desc.offset_x = static_cast<int32_t>(w.data1);
        window_desc.offset_y = static_cast<int32_t>(w.data2);
        result = WINDOW_PROCESS_FLAG_MOVED;
        break;

    case SDL_WINDOWEVENT_RESIZED:
        BUMA_LOGI("resized to {}x{} \"{}\"", w.data1, w.data2, window_title);
        window_desc.width = static_cast<uint32_t>(w.data1);
        window_desc.height = static_cast<uint32_t>(w.data2);
        aspect_ratio = static_cast<float>(window_desc.width) / static_cast<float>(window_desc.height);
        result = WINDOW_PROCESS_FLAG_RESIZED;
        break;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        break;

    case SDL_WINDOWEVENT_MINIMIZED:
        BUMA_LOGI("minimized \"{}\"", window_title);
        window_desc.state = WINDOW_STATE_MINIMIZE;
        result = WINDOW_PROCESS_FLAG_MINIMIZED;
        break;
    case SDL_WINDOWEVENT_MAXIMIZED:
        BUMA_LOGI("maximized \"{}\"", window_title);
        window_desc.state = WINDOW_STATE_MAXIMIZE;
        result = WINDOW_PROCESS_FLAG_MAXIMIZED;
        break;
    case SDL_WINDOWEVENT_RESTORED:
        BUMA_LOGI("restored \"{}\"", window_title);
        window_desc.state = WINDOW_STATE_RESTORE;
        result = WINDOW_PROCESS_FLAG_RESTORED;
        break;

    case SDL_WINDOWEVENT_ENTER:
        break;
    case SDL_WINDOWEVENT_LEAVE:
        break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        BUMA_LOGI("activated \"{}\"", window_title);
        is_window_active = true;
        result = WINDOW_PROCESS_FLAG_ACTIVATED;
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        BUMA_LOGI("deactivated \"{}\"", window_title);
        is_window_active = false;
        result = WINDOW_PROCESS_FLAG_DEACTIVATED;
        break;

    case SDL_WINDOWEVENT_CLOSE:
        BUMA_LOGI("should close \"{}\"", window_title);
        should_close = true;
        result = WINDOW_PROCESS_FLAG_CLOSE;
        break;
    case SDL_WINDOWEVENT_TAKE_FOCUS:
        break;
    case SDL_WINDOWEVENT_HIT_TEST:
        break;

    default:
        break;
    }
    result |= UpdateWindowStyles();
    return result;
}

WINDOW_PROCESS_FLAGS WindowSDL::UpdateWindowStyles()
{
    auto old_style = window_desc.style;
    auto old_flags = window_desc.flags;

    window_desc.flags = WINDOW_FLAG_NONE;
    auto flags = SDL_GetWindowFlags(sdlwindow);
    if (flags & SDL_WINDOW_RESIZABLE)     window_desc.flags |= WINDOW_FLAG_RESIZABLE;
    if (flags & SDL_WINDOW_ALWAYS_ON_TOP) window_desc.flags |= WINDOW_FLAG_ALWAYS_ON_TOP;

    window_desc.style = flags & SDL_WINDOW_BORDERLESS
        ? WINDOW_STYLE_BORDERLESS
        : WINDOW_STYLE_WINDOWED;

    if (window_desc.style != old_style)
    {
        BUMA_LOGI("style changed \"{}\"", window_title);
        return WINDOW_PROCESS_FLAG_STYLE_CHANGED;
    }
    return WINDOW_PROCESS_FLAG_NONE;
}

} // namespace buma
