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
    , timer         {}
    , is_prepared   {}
    , should_exit   {}
    , attached_app  {}
{
    cmd_lines = std::make_unique<std::vector<std::string>>();
}

PlatformBase::~PlatformBase()
{
    cmd_lines.reset();
}


}// namespace buma
