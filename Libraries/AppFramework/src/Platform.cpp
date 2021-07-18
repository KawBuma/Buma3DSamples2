#include <AppFramework/Platform.h>

namespace buma
{

const std::vector<std::string>& PlatformBase::GetCommandArguments()
{
    return *cmd_lines;
}

std::vector<std::string>::const_iterator PlatformBase::FindArgument(const char* _find_str)
{
    return std::find_if(cmd_lines->cbegin(), cmd_lines->cend(), [_find_str](const std::string& _str) {
        return _str == _find_str;
    });
}

bool PlatformBase::HasArgument(const char* _find_str)
{
    return std::find(cmd_lines->begin(), cmd_lines->end(), _find_str) != cmd_lines->end();
}

PlatformBase::PlatformBase()
    : cmd_lines     {}
    , help_message  {}
    , timer         {}
    , is_prepared   {}
    , should_exit   {}
    , attached_app  {}
{
    cmd_lines = std::make_unique<std::vector<std::string>>();
    help_message = std::make_unique<std::string>(
R"(USAGE: Buma3DSamples.exe Options <User Inputs>
Options:
--help, -h
    ヘルプメッセージを表示します。

--app <application path>
    起動するアプリケーションモジュールの、拡張子を含まないファイル名を指定します。
    for instance: --app HelloTriangle/HelloTriangle 

--enable-log
    ログのファイル出力を有効にします。

)");
}

PlatformBase::~PlatformBase()
{
    cmd_lines.reset();
    help_message.reset();
}


}// namespace buma
