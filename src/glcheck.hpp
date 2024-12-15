#pragma once

#include <filesystem>
#include <string_view>

#ifndef NDEBUG
#define glCheck(expr) do { expr; glCheckError(__FILE__, __LINE__, #expr); } while (0)
#else
#define glCheck(expr) do { expr; } while (0)
#endif

void glCheckError(const std::filesystem::path &file,
                  unsigned int line,
                  std::string_view expression);
