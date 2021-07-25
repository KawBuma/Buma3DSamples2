#include "PlatformSDL.h"

#include <AppFramework/Application.h>
#include "./WindowSDL.h"

#include <Utils/Definitions.h>

#define BUMA_MIN_LOGTYPE BUMA_LOGTYPE_ALL
#include <Utils/Logger.h>

#include <SDL.h>

#include <filesystem>
#include <iostream>
#include <tuple>

#define RET_IF_FAILED(x, result) { if (!x) return result; }

namespace buma
{

PlatformSDL::PlatformSDL(const PLATFORM_DESC& _desc)
    : PlatformBase  ()
    , windows       {}
    , applications  {}
{
    Init(_desc);
}

PlatformSDL::~PlatformSDL()
{
    Term();
}

bool PlatformSDL::Init(const PLATFORM_DESC& _desc)
{
    RET_IF_FAILED(ParseCommandLines(_desc), false);
    RET_IF_FAILED(PrepareLog(), false);
    BUMA_LOGI("Initialize Platform");

    SDL_SetMainReady();
    RET_IF_FAILED(SDL_Init(SDL_INIT_EVENTS) == 0, false);

    // 引数のdescをコピー
    auto&& _d = *reinterpret_cast<const PLATFORM_DATA_SDL*>(_desc.data);
    desc_data.argc = _d.argc;
    desc_data.argv = new const char* [_d.argc];
    for (int i = 0; i < _d.argc; i++)
        desc_data.argv[i] = cmd_lines->at(i).c_str();
    desc.type = _desc.type;
    desc.data = &desc_data;

    is_prepared = true;
    return true;
}

bool PlatformSDL::ParseCommandLines(const PLATFORM_DESC& _desc)
{
    BUMA_ASSERT(_desc.type == PLATFORM_TYPE_SDL);
    if (_desc.type != PLATFORM_TYPE_SDL)
        return false;
    auto&& d = *reinterpret_cast<const PLATFORM_DATA_SDL*>(_desc.data);
    cmd_lines->reserve(d.argc);
    for (int i = 0; i < d.argc; i++)
        cmd_lines->emplace_back(d.argv[i]);

    return true;
}

bool PlatformSDL::PrepareLog()
{
    if (IS_DEBUG || HasArgument("--enable-log"))
    {
        auto logger = log::create<log::sinks::basic_file_sink_mt>("buma", "logs/buma.log", true);
        log::set_default_logger(logger);

        // コンソールバッファにも出力
        auto console = std::make_shared<log::sinks::stdout_color_sink_mt>();
        logger->sinks().emplace_back(console);
        if constexpr (IS_DEBUG)
        {
            console->set_level(log::level::trace);
        }

        BUMA_LOGI("Logger [ buma ] initialized");
    }

    return true;
}

void PlatformSDL::Term()
{
    if (desc_data.argv)
    {
        delete[] desc_data.argv;
        desc_data.argv = nullptr;
    }
    BUMA_LOGI("Terminate Platform");
    BUMA_ASSERT(applications.empty() && "All applications must be destroyed before terminating platform");
    BUMA_ASSERT(windows.empty() && "All windows must be destroyed before terminating platform");
    SDL_Quit();
}

const PLATFORM_DESC& PlatformSDL::GetDesc() const
{
    return desc;
}

void PlatformSDL::PrintHelpMessage() const
{
    std::cout << *help_message << std::endl;
}

void PlatformSDL::AddHelpMessage(const char* _message)
{
    help_message->append(_message);
}

ApplicationBase* PlatformSDL::CreateApplication(const char* _path)
{
    void* module = SDL_LoadObject(_path);
    if (!module)
    {
        BUMA_LOGE("CreateApplication: failed to load application module {}", _path);
        return nullptr;
    }

    auto Create = reinterpret_cast<buma::PFN_CreateApplication>(SDL_LoadFunction(module, "CreateApplication"));
    ApplicationBase* result = Create(*this);
    if (!result->OnInit())
    {
        BUMA_LOGE("CreateApplication: failed to initialize application");

        auto Destroy = reinterpret_cast<buma::PFN_DestroyApplication>(SDL_LoadFunction(module, "DestroyApplication"));
        Destroy(result);

        SDL_UnloadObject(module);
        return nullptr;
    }

    applications[result] = static_cast<void*>(module);
    return result;
}

void PlatformSDL::DestroyApplication(ApplicationBase* _app)
{
    if (WasDestroyed(_app))
        return;

    auto it_find = applications.find(_app);
    BUMA_ASSERT(it_find != applications.end());

    _app->OnDestroy();
    _app->MarkAsDestroyed();

    // DestroyApplicationの呼び出しをApplicationBaseで行うケースためにモジュールの解放は遅延して行います。
    // 同時に解放を行うと、ApplicationBase関数内からのDestroyApplication呼び出しによってアクセス違反が発生します。
    pending_destroy.emplace_back([this, it_find](){
        auto Destroy = reinterpret_cast<buma::PFN_DestroyApplication>(SDL_LoadFunction(it_find->second, "DestroyApplication"));
        BUMA_ASSERT(Destroy);
        Destroy(it_find->first);
        SDL_UnloadObject(it_find->second);
        applications.erase(it_find);
    });
}

void PlatformSDL::AttachApplication(ApplicationBase* _app)
{
    if (attached_app)
    {
        attached_app->OnApplicatonDetached();
        attached_app = nullptr;
    }

    attached_app = _app;
    attached_app->OnApplicatonAttached();
}

WindowBase* PlatformSDL::CreateWindow(const WINDOW_DESC& _desc)
{
    auto result = new WindowSDL(*this, _desc);
    windows.emplace(reinterpret_cast<SDL_Window*>(result->GetNativeHandle()), result);
    return result;
}

void PlatformSDL::DestroyWindow(WindowBase* _window)
{
    auto&& it_find = windows.find(reinterpret_cast<SDL_Window*>(_window->GetNativeHandle()));
    BUMA_ASSERT(it_find != windows.end());
    delete static_cast<WindowSDL*>(_window);
    windows.erase(it_find);
}

int PlatformSDL::SDLCustomEventFilter(void* _userdata, SDL_Event* _event)
{
    auto&& t = *static_cast<PlatformSDL*>(_userdata);
    auto&& e = *_event;
    switch (_event->type)
    {
    case SDL_WINDOWEVENT:
        if (_event->window.event == SDL_WINDOWEVENT_RESIZED)
        {
            t.OnSDLWindowEvent(e);
            return 0;
        }
        break;

    case SDL_MOUSEMOTION:
        t.attached_app->OnMouseMove(MouseMoveEventArgs(&e.motion));
        return 0;
        break;

    default:
        break;
    }

    return 1;
}
int PlatformSDL::MainLoop()
{
    SDL_SetEventFilter(SDLCustomEventFilter, this);
    while (ProcessMessage())
    {
        timer.Tick();
        attached_app->Tick(timer);
        DestroyPendings();
    }

    if (attached_app)
    {
        attached_app->OnApplicatonDetached();
        if (!attached_app->WasDestroyed())
            DestroyApplication(attached_app);
        attached_app = nullptr;
    }
    RequestExit();
    DestroyPendings();
    return 0;
}

void PlatformSDL::RequestExit()
{
    BUMA_LOGI("Platform exit requested");
    should_exit = true;
    for (auto& [app, mod] : applications)
    {
        if (WasDestroyed(app)) continue;
        app->OnRequestPlatformExit();
    }
}

extern std::unordered_map<SDL_EventType, const char*> EM;

bool PlatformSDL::ProcessMessage()
{
    SDL_Event e{};
    while (SDL_PollEvent(&e) != 0)
    {
        //if (EM.find((SDL_EventType)e.type) != EM.end())
        //    log::trace(EM.at((SDL_EventType)e.type));

        switch (e.type)
        {
        case SDL_QUIT:
            should_exit = true;
            break;

        case SDL_DISPLAYEVENT:
            OnSDLDisplayEvent(e);
            break;

        case SDL_WINDOWEVENT:
            OnSDLWindowEvent(e);
            break;
        case SDL_SYSWMEVENT:
            OnSDLSystemEvent(e);
            break;

        case SDL_KEYDOWN:
            attached_app->OnKeyDown(KeyDownEventArgs(&e.key));
            break;
        case SDL_KEYUP:
            attached_app->OnKeyUp(KeyUpEventArgs(&e.key));
            break;
        case SDL_TEXTEDITING:
        case SDL_TEXTINPUT:
        case SDL_KEYMAPCHANGED:
            break;

        case SDL_MOUSEMOTION:
            attached_app->OnMouseMove(MouseMoveEventArgs(&e.motion));
            break;
        case SDL_MOUSEBUTTONDOWN:
            attached_app->OnMouseButtonDown(MouseButtonDownEventArgs(&e.button));
            break;
        case SDL_MOUSEBUTTONUP:
            attached_app->OnMouseButtonUp(MouseButtonUpEventArgs(&e.button));
            break;
        case SDL_MOUSEWHEEL:
            attached_app->OnMouseWheel(MouseWheelEventArgs(&e.wheel));
            break;

        case SDL_CLIPBOARDUPDATE:
            break;

        case SDL_DROPFILE:
        case SDL_DROPTEXT:
        case SDL_DROPBEGIN:
        case SDL_DROPCOMPLETE:
            break;

        case SDL_USEREVENT: // Events::SDL_USEREVENT through::SDL_LASTEVENT are for your use, and should be allocated with SDL_RegisterEvents()
            break;

        case SDL_LASTEVENT: // This last event is only for bounding internal arrays
            break;

        // case SDL_APP_TERMINATING:
        // case SDL_APP_LOWMEMORY:
        // case SDL_APP_WILLENTERBACKGROUND:
        // case SDL_APP_DIDENTERBACKGROUND:
        // case SDL_APP_WILLENTERFOREGROUND:
        // case SDL_APP_DIDENTERFOREGROUND:
        //     break;
        // case SDL_LOCALECHANGED:
        //     break;
        // case SDL_JOYAXISMOTION:
        // case SDL_JOYBALLMOTION:
        // case SDL_JOYHATMOTION:
        // case SDL_JOYBUTTONDOWN:
        // case SDL_JOYBUTTONUP:
        // case SDL_JOYDEVICEADDED:
        // case SDL_JOYDEVICEREMOVED:
        //     break;
        // case SDL_CONTROLLERAXISMOTION:
        // case SDL_CONTROLLERBUTTONDOWN:
        // case SDL_CONTROLLERBUTTONUP:
        // case SDL_CONTROLLERDEVICEADDED:
        // case SDL_CONTROLLERDEVICEREMOVED:
        // case SDL_CONTROLLERDEVICEREMAPPED:
        // case SDL_CONTROLLERTOUCHPADDOWN:
        // case SDL_CONTROLLERTOUCHPADMOTION:
        // case SDL_CONTROLLERTOUCHPADUP:
        // case SDL_CONTROLLERSENSORUPDATE:
        //     break;
        // case SDL_FINGERDOWN:
        // case SDL_FINGERUP:
        // case SDL_FINGERMOTION:
        //     break;
        // case SDL_DOLLARGESTURE: // $1 Unistroke Recognizer
        // case SDL_DOLLARRECORD:
        // case SDL_MULTIGESTURE:
        //     break;
        // case SDL_AUDIODEVICEADDED:
        // case SDL_AUDIODEVICEREMOVED:
        //     break;
        // case SDL_SENSORUPDATE:
        //     break;
        // case SDL_RENDER_TARGETS_RESET:
        // case SDL_RENDER_DEVICE_RESET:
        //     break;
        default:
            break;
        }

        auto a = ProcessMessageEventArgs::PROCESS_MESSAGE_EVENT_ARGS_SDL({ &e });
        attached_app->OnProcessMessage(ProcessMessageEventArgs(PLATFORM_TYPE_SDL, (void*)&a));
    }

    return !should_exit;
}

void PlatformSDL::OnSDLDisplayEvent(SDL_Event& e)
{
    BUMA_UNREDERENCED(e);
    //e.display.type;
    //e.display.timestamp;
    //e.display.display;
    //e.display.event;
    //e.display.data1;
    //auto&& d = e.display;
    //switch (d.type)
    //{
    //case SDL_DISPLAYEVENT_NONE:
    //    break;
    //case SDL_DISPLAYEVENT_ORIENTATION:
    //    break;
    //case SDL_DISPLAYEVENT_CONNECTED:
    //    break;
    //case SDL_DISPLAYEVENT_DISCONNECTED:
    //    break;
    //
    //default:
    //    break;
    //}
}

void PlatformSDL::OnSDLWindowEvent(SDL_Event& e)
{
    auto&& w = e.window;
    auto sdlwnd = SDL_GetWindowFromID(w.windowID);

    // 管理されていないウィンドウを無視します
    auto&& it_find = windows.find(sdlwnd);
    if (it_find == windows.end())
    {
        BUMA_LOGD("Ignore unmanaged WindowID {}", e.window.windowID);
        return;
    }

    auto wnd = it_find->second;
    ProcessWindowEventArgs args(wnd, wnd->OnSDLWindowEvent(e));
    for (auto& [app, m] : applications)
    {
        if (WasDestroyed(app)) continue;
        app->OnProcessWindow(args);
    }
}

void PlatformSDL::OnSDLSystemEvent(SDL_Event& e)
{
    BUMA_UNREDERENCED(e);
    // e.syswm.type;
    // e.syswm.timestamp;
    // e.syswm.msg;
}

void PlatformSDL::DestroyPendings()
{
    BUMA_LOGD("Destroying pending app");
    // DestroyApplicationで追加された保留中のモジュールを解放
    for (auto& destroy : pending_destroy)
    {
        destroy();
    }
    pending_destroy.clear();
}

bool PlatformSDL::WasDestroyed(const ApplicationBase* _app) const
{
    if (_app->WasDestroyed())
    {
        BUMA_LOGT("Ignore {} that has already been destroyed", _app->GetName());
        return true;
    }
    return false;
}

PlatformBase* PlatformBase::CreatePlatform(const PLATFORM_DESC& _desc)
{
    return new PlatformSDL(_desc);
}

void PlatformBase::DestroyPlatform(PlatformBase* _platform)
{
    delete _platform;
}

#define T(x) { SDL_##x, #x }
static std::unordered_map<SDL_EventType, const char*> EM = {
      T(QUIT)
    , T(APP_TERMINATING)
    , T(APP_LOWMEMORY)
    , T(APP_WILLENTERBACKGROUND)
    , T(APP_DIDENTERBACKGROUND)
    , T(APP_WILLENTERFOREGROUND)
    , T(APP_DIDENTERFOREGROUND)
    , T(LOCALECHANGED)
    , T(DISPLAYEVENT)
    , T(WINDOWEVENT)
    , T(SYSWMEVENT)
    , T(KEYDOWN)
    , T(KEYUP)
    , T(TEXTEDITING)
    , T(TEXTINPUT)
    , T(KEYMAPCHANGED)
    , T(MOUSEMOTION)
    , T(MOUSEBUTTONDOWN)
    , T(MOUSEBUTTONUP)
    , T(MOUSEWHEEL)
    , T(JOYAXISMOTION)
    , T(JOYBALLMOTION)
    , T(JOYHATMOTION)
    , T(JOYBUTTONDOWN)
    , T(JOYBUTTONUP)
    , T(JOYDEVICEADDED)
    , T(JOYDEVICEREMOVED)
    , T(CONTROLLERAXISMOTION)
    , T(CONTROLLERBUTTONDOWN)
    , T(CONTROLLERBUTTONUP)
    , T(CONTROLLERDEVICEADDED)
    , T(CONTROLLERDEVICEREMOVED)
    , T(CONTROLLERDEVICEREMAPPED)
    , T(CONTROLLERTOUCHPADDOWN)
    , T(CONTROLLERTOUCHPADMOTION)
    , T(CONTROLLERTOUCHPADUP)
    , T(CONTROLLERSENSORUPDATE)
    , T(FINGERDOWN)
    , T(FINGERUP)
    , T(FINGERMOTION)
    , T(DOLLARGESTURE)
    , T(DOLLARRECORD)
    , T(MULTIGESTURE)
    , T(CLIPBOARDUPDATE)
    , T(DROPFILE)
    , T(DROPTEXT)
    , T(DROPBEGIN)
    , T(DROPCOMPLETE)
    , T(AUDIODEVICEADDED)
    , T(AUDIODEVICEREMOVED)
    , T(SENSORUPDATE)
    , T(RENDER_TARGETS_RESET)
    , T(RENDER_DEVICE_RESET)
    , T(USEREVENT)
    , T(LASTEVENT)
};


} // namespace buma
