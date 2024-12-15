#pragma once

#include <filesystem>
#include <string>

namespace Utility
{
std::string loadFile(const std::filesystem::path &path);
}
