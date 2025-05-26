#pragma once

#include <filesystem>
#include <string>

namespace Utility
{
std::string loadFile(const std::filesystem::path &path);
std::u32string decodeUTF8(std::string_view str);
}
