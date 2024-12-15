#include <fstream>
#include <sstream>

#include "utility.hpp"

namespace Utility
{
std::string loadFile(const std::filesystem::path &path)
{
	std::ifstream in(path);
	if (!in)
	{
		throw std::runtime_error("cannot open " + path.string());
	}

	std::stringstream buffer;
	buffer << in.rdbuf();

	return buffer.str();
}
}
