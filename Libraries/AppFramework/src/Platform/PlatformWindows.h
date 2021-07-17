#pragma once

#include <AppFramework/Platform.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace buma
{

class WindowWindows;

class PlatformWindows : public PlatformBase
{
public:
    inline static constexpr const wchar_t* CLASS_NAME = L"Buma3DSamples Framework";

public:
    PlatformWindows();
    virtual ~PlatformWindows();

    int MainLoop() override;
    void ProcessMain();

    HINSTANCE GetHinstance() const { return hins; }

    bool Prepare(const PLATFORM_DESC& _desc) override;
    bool Init() override;
    bool Term() override;

protected:
    bool ParseCommandLines(const PLATFORM_DESC& _desc) override;
    bool PrepareWindow() override;
    bool RegisterWndClass();
    bool PrepareLog();

private:
    class ConsoleSession;
    WNDCLASSEXW                             wnd_class;
    HINSTANCE                               hins;
    HINSTANCE                               prev_hins;
    std::string                             cmdline;
    int                                     num_cmdshow;
    std::shared_ptr<WindowWindows>          window_windows;
    std::string                             execution_path;
    ConsoleSession*                         console_session;

};


}// namespace buma
