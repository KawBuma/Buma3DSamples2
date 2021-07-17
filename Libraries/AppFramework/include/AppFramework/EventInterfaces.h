#pragma once

#include <AppFramework/Platform.h>
#include <AppFramework/Window.h>

#include <Utils/LazyDelegate.h>

union SDL_Event;
struct SDL_KeyboardEvent;
struct SDL_MouseMotionEvent;
struct SDL_MouseButtonEvent;
struct SDL_MouseWheelEvent;

namespace buma
{

class WindowBase;

class ProcessWindowEventArgs;
class ProcessMessageEventArgs;
class KeyDownEventArgs;
class KeyUpEventArgs;
class MouseMoveEventArgs;
class MouseButtonDownEventArgs;
class MouseButtonUpEventArgs;
class MouseWheelEventArgs;

class ProcessWindowEventArgs : public IEventArgs
{
public:
    ProcessWindowEventArgs(const WindowBase* _window, WINDOW_PROCESS_FLAGS _process_flags)
        : window{ _window }, process_flags{ _process_flags }
    {}
    ~ProcessWindowEventArgs() {}

public:
    const WindowBase*           window;
    const WINDOW_PROCESS_FLAGS  process_flags;

};

class ProcessMessageEventArgs : public IEventArgs
{
public:
    struct PROCESS_MESSAGE_EVENT_ARGS_WINDOWS
    {
        void*               hwnd;
        uint32_t            message;
        unsigned long long  wparam;
        long long           lparam;
    };

    struct PROCESS_MESSAGE_EVENT_ARGS_SDL
    {
        const SDL_Event* event;
    };

public:
    ProcessMessageEventArgs(PLATFORM_TYPE _type, void* _data)
        : type          { _type }
        , data          { _data }
        , is_processed  { false }
    {}
    ~ProcessMessageEventArgs() {}

public:
    const PLATFORM_TYPE     type;
    const void*             data;
    bool                    is_processed; // 所定のイベントを処理したかどうかをIEvent内で設定するためのストレージです。 

};

class KeyDownEventArgs : public IEventArgs
{
public:
    KeyDownEventArgs(const SDL_KeyboardEvent* _key)
        : key{ _key }
    {}
    ~KeyDownEventArgs() {}

public:
    const SDL_KeyboardEvent* key;

};

class KeyUpEventArgs : public IEventArgs
{
public:
    KeyUpEventArgs(const SDL_KeyboardEvent* _key)
        : key{ _key }
    {}
    ~KeyUpEventArgs() {}

public:
    const SDL_KeyboardEvent* key;

};

class MouseMoveEventArgs : public IEventArgs
{
public:
    MouseMoveEventArgs(const SDL_MouseMotionEvent* _mouse)
        : mouse{ _mouse }
    {}
    ~MouseMoveEventArgs() {}

public:
    const SDL_MouseMotionEvent* mouse;

};

class MouseButtonDownEventArgs : public IEventArgs
{
public:
    MouseButtonDownEventArgs(const SDL_MouseButtonEvent* _mouse)
        : mouse{ _mouse }
    {}
    ~MouseButtonDownEventArgs() {}

public:
    const SDL_MouseButtonEvent* mouse;

};

class MouseButtonUpEventArgs : public IEventArgs
{
public:
    MouseButtonUpEventArgs(const SDL_MouseButtonEvent* _mouse)
        : mouse{ _mouse }
    {}
    ~MouseButtonUpEventArgs() {}

public:
    const SDL_MouseButtonEvent* mouse;

};

class MouseWheelEventArgs : public IEventArgs
{
public:
    MouseWheelEventArgs(const SDL_MouseWheelEvent* _wheel)
        : wheel{ _wheel }
    {}
    ~MouseWheelEventArgs() {}

public:
    const SDL_MouseWheelEvent* wheel;

};


} // namespace buma
