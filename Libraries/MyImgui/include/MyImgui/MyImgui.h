#pragma once
#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

#include <imgui.h>

#include <memory>

namespace buma
{
class DeviceResources;
class ProcessMessageEventArgs;
}

union SDL_Event;

namespace buma
{
namespace gui
{

enum MYIMGUI_CREATE_FLAG
{
      MYIMGUI_CREATE_FLAG_NONE                              = 0x0

    // このフラグが指定された場合、RecordGuiCommandsの呼び出しが可能です。 
    // ImGui::Image等による任意の画像用ディスクリプタプールを、必要に応じて追加作成します。
    // これにより、ディスクリプタの枯渇による描画コマンド実行の分割を回避できますが、メモリを過剰に消費する可能性があります。 
    , MYIMGUI_CREATE_FLAG_DESCRIPTOR_POOL_FEEDING           = 0x1

    // 各ビューポートのコマンドを全て RecordGuiCommands::_list 上で書き込みます。 
    // DESCRIPTOR_POOL_FEEDING も同時に含まれている必要があります。
    , MYIMGUI_CREATE_FLAG_USE_SINGLE_COMMAND_LIST           = 0x2
};
using MYIMGUI_CREATE_FLAGS = uint32_t;

struct MYIMGUI_CREATE_DESC
{
    void*                       window_handle; // HWND, SDL_Window*
    int                         config_flags;  // ImGuiConfigFlags
    buma3d::RESOURCE_FORMAT     framebuffer_format;
    MYIMGUI_CREATE_FLAGS        flags;
};

class MyImGui
{
public:
    MyImGui(DeviceResources* _dr);
    ~MyImGui();

    bool Init(const MYIMGUI_CREATE_DESC& _desc);

    void BeginFrame(buma3d::IView* _rtv);
    void EndFrame(buma3d::RESOURCE_STATE _current_state, buma3d::RESOURCE_STATE _state_after);
    void ProcessMessage(const SDL_Event* _event);
    // リサイズ時に呼び出す必要があります。 
    void ClearFbCache();

    void Destroy();

private:
    class MyImGuiImpl;
    MyImGuiImpl* impl;

};


}//namespace gui
}//namespace buma
