#include <AppFramework/Application.h>

#include <nlohmann/json.hpp>

#include <fstream>

namespace buma
{

ApplicationBase::ApplicationBase(PlatformBase& _platform)
    : platform      { _platform }
    , settings      {}
    , was_destroyed {}
{
}

ApplicationBase::~ApplicationBase()
{

}

bool ApplicationBase::PrepareSettings()
{
    std::ifstream ifs("b3dsample_settings.json");
    if (ifs.fail()) return false;

    auto&& s = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
    auto&& json = nlohmann::json::parse(s);

    auto&& o = json["Sample"];
    settings.asset_path             = o["ASSET_PATH"       ].get<std::string>();
    settings.window_desc.width      = o["WIDTH"            ].get<uint32_t>();
    settings.window_desc.height     = o["HEIGHT"           ].get<uint32_t>();
    settings.is_disabled_vsync      = o["DISABLE_VSYNC"    ].get<bool>();
    settings.is_enabled_fullscreen  = o["ENABLE_FULLSCREEN"].get<bool>();

    return true;
}


}// namespace buma

