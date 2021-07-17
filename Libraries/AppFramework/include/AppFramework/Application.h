#pragma once

#include <AppFramework/Platform.h>
#include <AppFramework/Window.h>
#include <AppFramework/EventInterfaces.h>

#include <Utils/Compiler.h>

#include <string>

#define BUMA_APP_APIENTRY __stdcall

#ifdef BUMA_APP_EXPORT
#define BUMA_APPMODULE_API extern "C" __declspec(dllexport)
#else
#define BUMA_APPMODULE_API extern "C" __declspec(dllimport)
#endif // BUMA_APP_EXPORT


namespace buma
{

struct APPLICATION_SETTINGS
{
    std::string     asset_path;
    WINDOW_DESC     window_desc;
    bool            is_disabled_vsync;
    bool            is_enabled_fullscreen;
};

class ApplicationBase
{
protected:
    ApplicationBase(PlatformBase& _platform);
    ApplicationBase(const ApplicationBase&) = delete;
    ApplicationBase& operator=(const ApplicationBase&) = delete;
    virtual ~ApplicationBase();

public:
    const APPLICATION_SETTINGS& GetSettings() const { return settings; }

    virtual void OnProcessWindow  (ProcessWindowEventArgs&   _args) { BUMA_UNREDERENCED(_args); }
    virtual void OnProcessMessage (ProcessMessageEventArgs&  _args) { BUMA_UNREDERENCED(_args); }
    virtual void OnKeyDown        (KeyDownEventArgs&         _args) { BUMA_UNREDERENCED(_args); }
    virtual void OnKeyUp          (KeyUpEventArgs&           _args) { BUMA_UNREDERENCED(_args); }
    virtual void OnMouseMove      (MouseMoveEventArgs&       _args) { BUMA_UNREDERENCED(_args); }
    virtual void OnMouseButtonDown(MouseButtonDownEventArgs& _args) { BUMA_UNREDERENCED(_args); }
    virtual void OnMouseButtonUp  (MouseButtonUpEventArgs&   _args) { BUMA_UNREDERENCED(_args); }
    virtual void OnMouseWheel     (MouseWheelEventArgs&      _args) { BUMA_UNREDERENCED(_args); }

    virtual bool IsAttached() const { return platform.GetAttachedApplication() == this; }
    virtual void OnApplicatonAttached() {}
    virtual void OnApplicatonDetached() { platform.DestroyApplication(this); }
    virtual void OnRequestPlatformExit() { platform.DestroyApplication(this); }

    virtual bool OnInit() = 0;
    virtual void Tick(const util::StepTimer& _timer) = 0;
    virtual void OnDestroy() = 0;
    virtual void MarkAsDestroyed() { was_destroyed = true; }
    virtual bool WasDestroyed() const { return was_destroyed; }

    virtual const char* GetName() const = 0;

protected:
    virtual bool PrepareSettings();
    std::string AssetPath(const char* _str) { return settings.asset_path + _str; }

protected:
    PlatformBase&        platform;
    APPLICATION_SETTINGS settings;

private:
    bool was_destroyed;

};

using PFN_CreateApplication = ApplicationBase* (BUMA_APP_APIENTRY*)(PlatformBase& _platform);
using PFN_DestroyApplication = void (BUMA_APP_APIENTRY*)(ApplicationBase* _app);


}// namespace buma
