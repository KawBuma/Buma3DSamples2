#include <AppFramework/Platform.h>

#include "./PlatformWindows.h"
#include "./WindowWindows.h"

#include <Utils/Utils.h>
#include <Utils/Logger.h>

#include <type_traits>
#include <unordered_map>

namespace buma
{

namespace /*anonymous*/
{

inline void ConvertToWindowSize(HWND _hwnd, uint32_t& _client_to_window_w, uint32_t& _client_to_window_h)
{
    RECT wr{}, cr{};
    GetWindowRect(_hwnd, &wr);
    GetClientRect(_hwnd, &cr);
    auto&& diff_w = uint32_t((wr.right - wr.left) - cr.right);
    auto&& diff_h = uint32_t((wr.bottom - wr.top) - cr.bottom);
    _client_to_window_w = _client_to_window_w + diff_w;
    _client_to_window_h = _client_to_window_h + diff_h;
}

inline void ConvertToWindowOffset(HWND _hwnd, int32_t& _client_to_window_x, int32_t& _client_to_window_y)
{
    RECT wr{};
    GetWindowRect(_hwnd, &wr);

    POINT offset{};
    ClientToScreen(_hwnd, &offset);
    _client_to_window_x = _client_to_window_x - (offset.x - wr.left);
    _client_to_window_y = _client_to_window_y - (offset.y - wr.top);
}

inline constexpr LONG_PTR ConvertWindowStyle(WINDOW_STYLE _style, WINDOW_FLAGS _flags)
{
    LONG_PTR result{};
    switch (_style)
    {
    case buma::WINDOW_STYLE_WINDOWED   : result = WS_VISIBLE | (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME);
    case buma::WINDOW_STYLE_BORDERLESS : result = WS_VISIBLE | WS_POPUP;
    default:
        result = WS_VISIBLE | (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME);
    }
    if (_flags & WINDOW_FLAG_RESIZABLE)
        result |= WS_THICKFRAME;
    return result;
}

inline constexpr int ConvertWindowState(WINDOW_STATE _state)
{
    switch (_state)
    {
    case buma::WINDOW_STATE_RESTORE   : return SW_RESTORE;
    case buma::WINDOW_STATE_MAXIMIZE  : return SW_MAXIMIZE;
    case buma::WINDOW_STATE_MINIMIZE  : return SW_MINIMIZE;
    default:
        return SW_RESTORE;
    }
}

}// namespace /*anonymous*/


WindowWindows::WindowWindows(PlatformWindows& _platform, WNDCLASSEXW& _wnd_class)
    : WindowBase()
    , platform                      { _platform }
    , wnd_class                     { _wnd_class }
    , hwnd                          {}
    , window_title                  {}
    , window_desc                   {}
    , aspect_ratio                  {}
    , window_process_flags          {}
    , is_window_active              {}
    , should_exit                   {}
    , msg                           {}
    , delegate_on_resize            {}
    , delegate_on_process_message   {}
    , delegate_on_process_window    {}
{
}

WindowWindows::~WindowWindows()
{
    if (hwnd)
        DestroyWindow(hwnd);
    hwnd = NULL;
}

bool WindowWindows::SetWindowTitle(const char* _text)
{
    window_title = _text ? _text : "";
    window_desc.window_title = window_title.c_str();
    return SetWindowTextA(hwnd, _text);
}

bool WindowWindows::OffsetWindowToCenter()
{
    uint32_t mw, mh;
    GetNearestMonitorResolution(&mw, &mh);

    uint32_t x = (mw / 2) - (window_desc.width  / 2);
    uint32_t y = (mh / 2) - (window_desc.height / 2);
    OffsetWindow(x, y);
    return true;
}

bool WindowWindows::OffsetWindow(int32_t _x, int32_t _y)
{
    ConvertToWindowOffset(hwnd, _x, _y);
    SetWindowPos(hwnd, NULL, _x, _y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    return true;
}

bool WindowWindows::ResizeWindow(uint32_t _w, uint32_t _h)
{
    ConvertToWindowSize(hwnd, _w, _h);
    SetWindowPos(hwnd, NULL, 0, 0, static_cast<int>(_w), static_cast<int>(_h), SWP_NOMOVE | SWP_NOZORDER);
    return true;
}

bool WindowWindows::SetWindowStyle(WINDOW_STYLE _style)
{
    SetWindowLongPtr(hwnd, GWL_STYLE, ConvertWindowStyle(_style, window_desc.flags));
    window_desc.style = _style;

    // 以下のフラグを指定したSetWindowPosの呼び出しによりSetWindowLongPtrの変更のみを適用します。 
    SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    return true;
}

bool WindowWindows::SetWindowState(WINDOW_STATE _state)
{
    if (is_window_active && window_desc.style == _state)
        return true;

    ShowWindow(hwnd, ConvertWindowState(_state));
    window_desc.state = _state;

    return true;
}

bool WindowWindows::SetWindowActive(bool _active)
{
    is_window_active = _active;
    if (_active)
        ShowWindow(hwnd, SW_SHOW);
    else
        SetActiveWindow(GetNextWindow(hwnd, GW_HWNDPREV));
    return true;
}

bool WindowWindows::IsWindowActive() const
{
    return is_window_active;
}

bool WindowWindows::ProcessMessage()
{
    if (should_exit)
        return false;

    window_process_flags = WINDOW_PROCESS_FLAG_NONE;
    // キューの残りメッセージがゼロになるまで取得
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            should_exit = true;
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //auto&& i = *platform.GetInputsImpl();
    //auto dt = platform.GetStepTimer().GetElapsedSeconds();
    //i.GetKeyboardInput().Update(dt);
    //i.GetMouseInput().Update(dt);
    //i.GetGamePadInputs().Update(dt);

    if (window_process_flags != WINDOW_PROCESS_FLAG_NONE)
    {
        auto args = ProcessWindowEventArgs(window_desc.state, window_desc.style, window_process_flags);
        delegate_on_process_window(&args);
    }

    return true;
}

bool WindowWindows::Exit()
{
    PostQuitMessage(0);
    return true;
}

void WindowWindows::AddResizeEvent(const EventPtr& _event)
{
    delegate_on_resize += _event;
}

void WindowWindows::RemoveResizeEvent(const EventPtr& _to_remove)
{
    delegate_on_resize -= _to_remove;
}

void WindowWindows::AddProcessMessageEvent(const EventPtr& _event)
{
    delegate_on_process_message += _event;
}

void WindowWindows::RemoveProcessMessageEvent(const EventPtr& _to_remove)
{
    delegate_on_process_message -= _to_remove;
}

void WindowWindows::AddProcessWindowEvent(const EventPtr& _event)
{
    delegate_on_process_window += _event;
}

void WindowWindows::RemoveProcessWindowEvent(const EventPtr& _to_remove)
{
    delegate_on_process_window -= _to_remove;
}

const char* WindowWindows::GetWindowedTitle() const
{
    return window_title.c_str();
}

bool WindowWindows::Init(PlatformBase& _platform, const WINDOW_DESC& _desc)
{
    window_desc = _desc;
    if (!_desc.need_window)
        return true;

    aspect_ratio = float(_desc.width) / float(_desc.height);

    if (!CreateWnd(_desc.width, _desc.height))  return false;
    if (!SetWindowTitle(_desc.window_title))    return false;

    return true;
}

bool WindowWindows::CreateWnd(uint32_t _width, uint32_t _height)
{
    DWORD style = ConvertWindowStyle(window_desc.style, window_desc.flags);
    RECT window_rect = { window_desc.offset_x, window_desc.offset_y, (LONG)_width, (LONG)_height };
    AdjustWindowRect(&window_rect, style, FALSE);

    hwnd = CreateWindowEx(0
                          , wnd_class.lpszClassName, util::ConvertAnsiToWide(window_title).c_str()
                          , style
                          , CW_USEDEFAULT, CW_USEDEFAULT
                          , window_rect.right - window_rect.left
                          , window_rect.bottom - window_rect.top
                          , nullptr, nullptr
                          , platform.GetHinstance(), reinterpret_cast<void*>(this));

    if (!hwnd)
        return false;

    //platform.GetInputsImpl()->GetMouseInput().SetWindow(hwnd);

    return true;
}

bool WindowWindows::OnResize(ResizeEventArgs* _args)
{
    delegate_on_resize(_args);

    window_desc.offset_x  = _args->new_offset_x;
    window_desc.offset_y  = _args->new_offset_y;
    window_desc.width     = _args->new_width;
    window_desc.height    = _args->new_height;
    aspect_ratio          = float(_args->new_width) / float(_args->new_height);

    return true;
}

WINDOW_STATE WindowWindows::GetWindowStateFromHwnd()
{
         if (IsZoomed(hwnd) == TRUE) return WINDOW_STATE_MAXIMIZE;
    else if (IsIconic(hwnd) == TRUE) return WINDOW_STATE_MINIMIZE;
    else                             return WINDOW_STATE_RESTORE;
}

WINDOW_STYLE WindowWindows::GetWindowStyleFromHwnd()
{
    auto styles = GetWindowLongPtr(hwnd, GWL_STYLE);
    if (styles & WS_POPUP)
        return WINDOW_STYLE_BORDERLESS;

    return WINDOW_STYLE_WINDOWED;
}

void WindowWindows::GetWindowOffsetFromHwnd(int32_t* _client_x, int32_t* _client_y)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    *_client_x = rc.left;
    *_client_y = rc.top;
}

void WindowWindows::GetWindowSizeFromHwnd(uint32_t* _client_w, uint32_t* _client_h)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    *_client_w = static_cast<uint32_t>(rc.right  - rc.left);
    *_client_h = static_cast<uint32_t>(rc.bottom - rc.top);
}

void WindowWindows::GetNearestMonitorResolution(uint32_t* _w, uint32_t* _h)
{
    HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEX mi{};
    mi.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(mon, &mi);
    *_w = static_cast<uint32_t>(mi.rcMonitor.right - mi.rcMonitor.left);
    *_h = static_cast<uint32_t>(mi.rcMonitor.bottom - mi.rcMonitor.top);
}

#define ENABLE_PRINT_WINDOW_MESSAGE
#ifdef ENABLE_PRINT_WINDOW_MESSAGE
static const std::unordered_map<UINT/*message*/, const char*> MESSAGE_NAMES_MAP;
#endif // ENABLE_PRINT_WINDOW_MESSAGE

LRESULT CALLBACK WindowWindows::WndProc(HWND _hwnd, UINT _message, WPARAM _wparam, LPARAM _lparam)
{
    auto fw = reinterpret_cast<WindowWindows*>(GetWindowLongPtr(_hwnd, GWLP_USERDATA));
    if (fw)
    {
        auto args_data = ProcessMessageEventArgs::PROCESS_MESSAGE_EVENT_ARGS_WINDOWS{ _hwnd, _message, _wparam, _lparam };
        ProcessMessageEventArgs args(PLATFORM_TYPE_WINDOWS, &args_data);
        fw->delegate_on_process_message(&args);
        if (args.is_processed)
            return 0;
    }

#ifdef ENABLE_PRINT_WINDOW_MESSAGE
    if (fw && MESSAGE_NAMES_MAP.find(_message) != MESSAGE_NAMES_MAP.end())
        log::debug(MESSAGE_NAMES_MAP.at(_message));
#endif // ENABLE_PRINT_WINDOW_MESSAGE

    switch (_message)
    {
    case WM_STYLECHANGED:
    {
        log::debug("window style changed");
        fw->window_process_flags |= WINDOW_PROCESS_FLAG_STYLE_CHANGED;
        break;
    }
    case WM_SIZE:
    {
        switch (_wparam)
        {
        case SIZE_RESTORED:
            log::debug("window restored");
            fw->window_process_flags |= WINDOW_PROCESS_FLAG_RESTORED;
            fw->window_desc.state = WINDOW_STATE_RESTORE;
            break;
        case SIZE_MINIMIZED:
            log::debug("window minimized");
            fw->window_process_flags |= WINDOW_PROCESS_FLAG_MINIMIZED;
            fw->window_desc.state = WINDOW_STATE_MINIMIZE;
            break;
        case SIZE_MAXIMIZED:
            log::debug("window maximized");
            fw->window_process_flags |= WINDOW_PROCESS_FLAG_MAXIMIZED;
            fw->window_desc.state = WINDOW_STATE_MAXIMIZE;
            break;
        case SIZE_MAXSHOW:
            log::debug("window maxshow");
            break;
        case SIZE_MAXHIDE:
            log::debug("window maxhide");
            break;
        default:
            break;
        }

        auto args = ResizeEventArgs(  fw->window_desc.offset_x , fw->window_desc.offset_y , fw->window_desc.width, fw->window_desc.height
                                    , uint32_t(LOWORD(_lparam)), uint32_t(HIWORD(_lparam)), fw->window_desc.width, fw->window_desc.height);
        fw->OnResize(&args);
        break;
    }
    case WM_MOVE:
    {
        auto args = ResizeEventArgs(  fw->window_desc.offset_x, fw->window_desc.offset_y, fw->window_desc.width, fw->window_desc.height
                                    , int32_t(LOWORD(_lparam)), int32_t(HIWORD(_lparam)), fw->window_desc.width, fw->window_desc.height);
        fw->OnResize(&args);
        break;
    }
    case WM_ENTERSIZEMOVE:
    {
        fw->window_process_flags |= WINDOW_PROCESS_FLAG_BEGIN_SIZEMOVE;
        log::debug("enter sizemove");
        break;
    }
    case WM_EXITSIZEMOVE:
    {
        fw->window_process_flags |= WINDOW_PROCESS_FLAG_END_SIZEMOVE;
        log::debug("exit sizemove");
        break;
    }
    case WM_GETMINMAXINFO:
    {
        auto info = reinterpret_cast<MINMAXINFO*>(_lparam);
        info->ptMinTrackSize.x = 80;
        info->ptMinTrackSize.y = 80;
        break;
    }
    case WM_DESTROY:
    {
        if (fw)
        {
            fw->window_process_flags |= WINDOW_PROCESS_FLAG_EXIT;
            fw->should_exit = true;
            log::info("exit");
        }
        break;
    }
    case WM_ACTIVATE:
    {
        //auto&& i = fw->platform.GetInputsImpl();
        //i->GetKeyboardInput().ProcessMessage(_message, _wparam, _lparam);
        //i->GetMouseInput().ProcessMessage(_message, _wparam, _lparam);
        //i->GetGamePadInputs();
        switch (_wparam)
        {
        case WA_ACTIVE:
            log::debug("window activated");
            fw->window_process_flags |= WINDOW_PROCESS_FLAG_ACTIVATED;
            fw->is_window_active = true;
            break;
        case WA_CLICKACTIVE:
            log::debug("window click activated");
            fw->window_process_flags |= WINDOW_PROCESS_FLAG_ACTIVATED;
            fw->is_window_active = true;
            break;
        case WA_INACTIVE:
            log::debug("window deactivated");
            fw->window_process_flags |= WINDOW_PROCESS_FLAG_DEACTIVATED;
            fw->is_window_active = false;
            break;
        default:
            break;
        }
        break;
    }
    case WM_CREATE:
    {
        LPCREATESTRUCT create_struct = reinterpret_cast<LPCREATESTRUCT>(_lparam);
        SetWindowLongPtr(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
        ShowWindow(_hwnd, SW_SHOWNORMAL);
        break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        //fw->platform.GetInputsImpl()->GetKeyboardInput().ProcessMessage(_message, _wparam, _lparam);
        break;
    }
    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
    {
        //fw->platform.GetInputsImpl()->GetMouseInput().ProcessMessage(_message, _wparam, _lparam);
        break;
    }

    default:
        // WndProc関数内で処理しないメッセージの場合、DefWindowProcを返すことでシステムがデフォルトの処理を行います。
        return DefWindowProc(_hwnd, _message, _wparam, _lparam);
    }

    // WndProc関数内で処理を行った(処理を実装している)メッセージは、0を返します。
    return 0;
}


}// namespace buma


#if defined ENABLE_PRINT_WINDOW_MESSAGE

#define KEY_NAME(x) { x, #x }
static const std::unordered_map<UINT/*message*/, const char*> MESSAGE_NAMES_MAP = {
      KEY_NAME(WM_NULL)
    , KEY_NAME(WM_CREATE)
    , KEY_NAME(WM_DESTROY)
    , KEY_NAME(WM_MOVE)
    , KEY_NAME(WM_SIZE)
    , KEY_NAME(WM_ACTIVATE)
    , KEY_NAME(WM_SETFOCUS)
    , KEY_NAME(WM_KILLFOCUS)
    , KEY_NAME(WM_ENABLE)
    , KEY_NAME(WM_SETREDRAW)
    , KEY_NAME(WM_SETTEXT)
    , KEY_NAME(WM_GETTEXT)
    , KEY_NAME(WM_GETTEXTLENGTH)
    , KEY_NAME(WM_PAINT)
    , KEY_NAME(WM_CLOSE)
    , KEY_NAME(WM_QUERYENDSESSION)
    , KEY_NAME(WM_QUERYOPEN)
    , KEY_NAME(WM_ENDSESSION)
    , KEY_NAME(WM_QUIT)
    , KEY_NAME(WM_ERASEBKGND)
    , KEY_NAME(WM_SYSCOLORCHANGE)
    , KEY_NAME(WM_SHOWWINDOW)
    , KEY_NAME(WM_WININICHANGE)
    , KEY_NAME(WM_SETTINGCHANGE)
    , KEY_NAME(WM_DEVMODECHANGE)
    , KEY_NAME(WM_ACTIVATEAPP)
    , KEY_NAME(WM_FONTCHANGE)
    , KEY_NAME(WM_TIMECHANGE)
    , KEY_NAME(WM_CANCELMODE)
    , KEY_NAME(WM_SETCURSOR)
    , KEY_NAME(WM_MOUSEACTIVATE)
    , KEY_NAME(WM_CHILDACTIVATE)
    , KEY_NAME(WM_QUEUESYNC)
    , KEY_NAME(WM_GETMINMAXINFO)
    , KEY_NAME(WM_PAINTICON)
    , KEY_NAME(WM_ICONERASEBKGND)
    , KEY_NAME(WM_NEXTDLGCTL)
    , KEY_NAME(WM_SPOOLERSTATUS)
    , KEY_NAME(WM_DRAWITEM)
    , KEY_NAME(WM_MEASUREITEM)
    , KEY_NAME(WM_DELETEITEM)
    , KEY_NAME(WM_VKEYTOITEM)
    , KEY_NAME(WM_CHARTOITEM)
    , KEY_NAME(WM_SETFONT)
    , KEY_NAME(WM_GETFONT)
    , KEY_NAME(WM_SETHOTKEY)
    , KEY_NAME(WM_GETHOTKEY)
    , KEY_NAME(WM_QUERYDRAGICON)
    , KEY_NAME(WM_COMPAREITEM)
    , KEY_NAME(WM_GETOBJECT)
    , KEY_NAME(WM_COMPACTING)
    , KEY_NAME(WM_COMMNOTIFY)
    , KEY_NAME(WM_WINDOWPOSCHANGING)
    , KEY_NAME(WM_WINDOWPOSCHANGED)
    , KEY_NAME(WM_POWER)
    , KEY_NAME(WM_COPYDATA)
    , KEY_NAME(WM_CANCELJOURNAL)
    , KEY_NAME(WM_NOTIFY)
    , KEY_NAME(WM_INPUTLANGCHANGEREQUEST)
    , KEY_NAME(WM_INPUTLANGCHANGE)
    , KEY_NAME(WM_TCARD)
    , KEY_NAME(WM_HELP)
    , KEY_NAME(WM_USERCHANGED)
    , KEY_NAME(WM_NOTIFYFORMAT)
    , KEY_NAME(WM_CONTEXTMENU)
    , KEY_NAME(WM_STYLECHANGING)
    , KEY_NAME(WM_STYLECHANGED)
    , KEY_NAME(WM_DISPLAYCHANGE)
    , KEY_NAME(WM_GETICON)
    , KEY_NAME(WM_SETICON)
    , KEY_NAME(WM_NCCREATE)
    , KEY_NAME(WM_NCDESTROY)
    , KEY_NAME(WM_NCCALCSIZE)
    , KEY_NAME(WM_NCHITTEST)
    , KEY_NAME(WM_NCPAINT)
    , KEY_NAME(WM_NCACTIVATE)
    , KEY_NAME(WM_GETDLGCODE)
    , KEY_NAME(WM_SYNCPAINT)
    , KEY_NAME(WM_NCMOUSEMOVE)
    , KEY_NAME(WM_NCLBUTTONDOWN)
    , KEY_NAME(WM_NCLBUTTONUP)
    , KEY_NAME(WM_NCLBUTTONDBLCLK)
    , KEY_NAME(WM_NCRBUTTONDOWN)
    , KEY_NAME(WM_NCRBUTTONUP)
    , KEY_NAME(WM_NCRBUTTONDBLCLK)
    , KEY_NAME(WM_NCMBUTTONDOWN)
    , KEY_NAME(WM_NCMBUTTONUP)
    , KEY_NAME(WM_NCMBUTTONDBLCLK)
    , KEY_NAME(WM_NCXBUTTONDOWN)
    , KEY_NAME(WM_NCXBUTTONUP)
    , KEY_NAME(WM_NCXBUTTONDBLCLK)
    , KEY_NAME(WM_INPUT_DEVICE_CHANGE)
    , KEY_NAME(WM_INPUT)
    , KEY_NAME(WM_KEYFIRST)
    , KEY_NAME(WM_KEYDOWN)
    , KEY_NAME(WM_KEYUP)
    , KEY_NAME(WM_CHAR)
    , KEY_NAME(WM_DEADCHAR)
    , KEY_NAME(WM_SYSKEYDOWN)
    , KEY_NAME(WM_SYSKEYUP)
    , KEY_NAME(WM_SYSCHAR)
    , KEY_NAME(WM_SYSDEADCHAR)
    , KEY_NAME(WM_UNICHAR)
    , KEY_NAME(WM_KEYLAST)
    , KEY_NAME(WM_KEYLAST)
    , KEY_NAME(WM_IME_STARTCOMPOSITION)
    , KEY_NAME(WM_IME_ENDCOMPOSITION)
    , KEY_NAME(WM_IME_COMPOSITION)
    , KEY_NAME(WM_IME_KEYLAST)
    , KEY_NAME(WM_INITDIALOG)
    , KEY_NAME(WM_COMMAND)
    , KEY_NAME(WM_SYSCOMMAND)
    , KEY_NAME(WM_TIMER)
    , KEY_NAME(WM_HSCROLL)
    , KEY_NAME(WM_VSCROLL)
    , KEY_NAME(WM_INITMENU)
    , KEY_NAME(WM_INITMENUPOPUP)
    , KEY_NAME(WM_GESTURE)
    , KEY_NAME(WM_GESTURENOTIFY)
    , KEY_NAME(WM_MENUSELECT)
    , KEY_NAME(WM_MENUCHAR)
    , KEY_NAME(WM_ENTERIDLE)
    , KEY_NAME(WM_MENURBUTTONUP)
    , KEY_NAME(WM_MENUDRAG)
    , KEY_NAME(WM_MENUGETOBJECT)
    , KEY_NAME(WM_UNINITMENUPOPUP)
    , KEY_NAME(WM_MENUCOMMAND)
    , KEY_NAME(WM_CHANGEUISTATE)
    , KEY_NAME(WM_UPDATEUISTATE)
    , KEY_NAME(WM_QUERYUISTATE)
    , KEY_NAME(WM_CTLCOLORMSGBOX)
    , KEY_NAME(WM_CTLCOLOREDIT)
    , KEY_NAME(WM_CTLCOLORLISTBOX)
    , KEY_NAME(WM_CTLCOLORBTN)
    , KEY_NAME(WM_CTLCOLORDLG)
    , KEY_NAME(WM_CTLCOLORSCROLLBAR)
    , KEY_NAME(WM_CTLCOLORSTATIC)
    , KEY_NAME(WM_MOUSEFIRST)
    , KEY_NAME(WM_MOUSEMOVE)
    , KEY_NAME(WM_LBUTTONDOWN)
    , KEY_NAME(WM_LBUTTONUP)
    , KEY_NAME(WM_LBUTTONDBLCLK)
    , KEY_NAME(WM_RBUTTONDOWN)
    , KEY_NAME(WM_RBUTTONUP)
    , KEY_NAME(WM_RBUTTONDBLCLK)
    , KEY_NAME(WM_MBUTTONDOWN)
    , KEY_NAME(WM_MBUTTONUP)
    , KEY_NAME(WM_MBUTTONDBLCLK)
    , KEY_NAME(WM_MOUSEWHEEL)
    , KEY_NAME(WM_XBUTTONDOWN)
    , KEY_NAME(WM_XBUTTONUP)
    , KEY_NAME(WM_XBUTTONDBLCLK)
    , KEY_NAME(WM_MOUSEHWHEEL)
    , KEY_NAME(WM_MOUSELAST)
    , KEY_NAME(WM_MOUSELAST)
    , KEY_NAME(WM_MOUSELAST)
    , KEY_NAME(WM_MOUSELAST)
    , KEY_NAME(WM_PARENTNOTIFY)
    , KEY_NAME(WM_ENTERMENULOOP)
    , KEY_NAME(WM_EXITMENULOOP)
    , KEY_NAME(WM_NEXTMENU)
    , KEY_NAME(WM_SIZING)
    , KEY_NAME(WM_CAPTURECHANGED)
    , KEY_NAME(WM_MOVING)
    , KEY_NAME(WM_POWERBROADCAST)
    , KEY_NAME(WM_DEVICECHANGE)
    , KEY_NAME(WM_MDICREATE)
    , KEY_NAME(WM_MDIDESTROY)
    , KEY_NAME(WM_MDIACTIVATE)
    , KEY_NAME(WM_MDIRESTORE)
    , KEY_NAME(WM_MDINEXT)
    , KEY_NAME(WM_MDIMAXIMIZE)
    , KEY_NAME(WM_MDITILE)
    , KEY_NAME(WM_MDICASCADE)
    , KEY_NAME(WM_MDIICONARRANGE)
    , KEY_NAME(WM_MDIGETACTIVE)
    , KEY_NAME(WM_MDISETMENU)
    , KEY_NAME(WM_ENTERSIZEMOVE)
    , KEY_NAME(WM_EXITSIZEMOVE)
    , KEY_NAME(WM_DROPFILES)
    , KEY_NAME(WM_MDIREFRESHMENU)
    , KEY_NAME(WM_POINTERDEVICECHANGE)
    , KEY_NAME(WM_POINTERDEVICEINRANGE)
    , KEY_NAME(WM_POINTERDEVICEOUTOFRANGE)
    , KEY_NAME(WM_TOUCH)
    , KEY_NAME(WM_NCPOINTERUPDATE)
    , KEY_NAME(WM_NCPOINTERDOWN)
    , KEY_NAME(WM_NCPOINTERUP)
    , KEY_NAME(WM_POINTERUPDATE)
    , KEY_NAME(WM_POINTERDOWN)
    , KEY_NAME(WM_POINTERUP)
    , KEY_NAME(WM_POINTERENTER)
    , KEY_NAME(WM_POINTERLEAVE)
    , KEY_NAME(WM_POINTERACTIVATE)
    , KEY_NAME(WM_POINTERCAPTURECHANGED)
    , KEY_NAME(WM_TOUCHHITTESTING)
    , KEY_NAME(WM_POINTERWHEEL)
    , KEY_NAME(WM_POINTERHWHEEL)
    , KEY_NAME(WM_POINTERROUTEDTO)
    , KEY_NAME(WM_POINTERROUTEDAWAY)
    , KEY_NAME(WM_POINTERROUTEDRELEASED)
    , KEY_NAME(WM_IME_SETCONTEXT)
    , KEY_NAME(WM_IME_NOTIFY)
    , KEY_NAME(WM_IME_CONTROL)
    , KEY_NAME(WM_IME_COMPOSITIONFULL)
    , KEY_NAME(WM_IME_SELECT)
    , KEY_NAME(WM_IME_CHAR)
    , KEY_NAME(WM_IME_REQUEST)
    , KEY_NAME(WM_IME_KEYDOWN)
    , KEY_NAME(WM_IME_KEYUP)
    , KEY_NAME(WM_MOUSEHOVER)
    , KEY_NAME(WM_MOUSELEAVE)
    , KEY_NAME(WM_NCMOUSEHOVER)
    , KEY_NAME(WM_NCMOUSELEAVE)
    , KEY_NAME(WM_WTSSESSION_CHANGE)
    , KEY_NAME(WM_TABLET_FIRST)
    , KEY_NAME(WM_TABLET_LAST)
    , KEY_NAME(WM_DPICHANGED)
    , KEY_NAME(WM_DPICHANGED_BEFOREPARENT)
    , KEY_NAME(WM_DPICHANGED_AFTERPARENT)
    , KEY_NAME(WM_GETDPISCALEDSIZE)
    , KEY_NAME(WM_CUT)
    , KEY_NAME(WM_COPY)
    , KEY_NAME(WM_PASTE)
    , KEY_NAME(WM_CLEAR)
    , KEY_NAME(WM_UNDO)
    , KEY_NAME(WM_RENDERFORMAT)
    , KEY_NAME(WM_RENDERALLFORMATS)
    , KEY_NAME(WM_DESTROYCLIPBOARD)
    , KEY_NAME(WM_DRAWCLIPBOARD)
    , KEY_NAME(WM_PAINTCLIPBOARD)
    , KEY_NAME(WM_VSCROLLCLIPBOARD)
    , KEY_NAME(WM_SIZECLIPBOARD)
    , KEY_NAME(WM_ASKCBFORMATNAME)
    , KEY_NAME(WM_CHANGECBCHAIN)
    , KEY_NAME(WM_HSCROLLCLIPBOARD)
    , KEY_NAME(WM_QUERYNEWPALETTE)
    , KEY_NAME(WM_PALETTEISCHANGING)
    , KEY_NAME(WM_PALETTECHANGED)
    , KEY_NAME(WM_HOTKEY)
    , KEY_NAME(WM_PRINT)
    , KEY_NAME(WM_PRINTCLIENT)
    , KEY_NAME(WM_APPCOMMAND)
    , KEY_NAME(WM_THEMECHANGED)
    , KEY_NAME(WM_CLIPBOARDUPDATE)
    , KEY_NAME(WM_DWMCOMPOSITIONCHANGED)
    , KEY_NAME(WM_DWMNCRENDERINGCHANGED)
    , KEY_NAME(WM_DWMCOLORIZATIONCOLORCHANGED)
    , KEY_NAME(WM_DWMWINDOWMAXIMIZEDCHANGE)
    , KEY_NAME(WM_DWMSENDICONICTHUMBNAIL)
    , KEY_NAME(WM_DWMSENDICONICLIVEPREVIEWBITMAP)
    , KEY_NAME(WM_GETTITLEBARINFOEX)
    , KEY_NAME(WM_HANDHELDFIRST)
    , KEY_NAME(WM_HANDHELDLAST)
    , KEY_NAME(WM_AFXFIRST)
    , KEY_NAME(WM_AFXLAST)
    , KEY_NAME(WM_PENWINFIRST)
    , KEY_NAME(WM_PENWINLAST)
    , KEY_NAME(WM_APP)
    , KEY_NAME(WM_USER)
};

#undef KEY_NAME 

#endif // ENABLE_PRINT_WINDOW_MESSAGE
