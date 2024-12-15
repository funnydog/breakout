#include <iostream>
#include <sstream>
#include <fstream>

#include "resource.h"
#include "stb_image.h"

std::map<std::string, Shader> ResourceManager::Shaders;
std::map<std::string, Texture2D> ResourceManager::Textures;

int
ResourceManager::LoadShader(std::string name, const char *vpath, const char *fpath, const char *gpath)
{
	Shader shader;
	if (!shader.create()) {
		std::cerr << "SHADER PROGRAM:\n";
		return -1;
	}

	std::string text;
	if (loadText(vpath, text) < 0
	    || !shader.attachString(Shader::Type::VERTEX, text)) {
		std::cerr << "VERTEX SHADER: " << vpath << "\n";
		shader.destroy();
		return -1;
	}

	if (loadText(fpath, text) < 0
	    || !shader.attachString(Shader::Type::FRAGMENT, text))
	{
		std::cerr << "FRAGMENT SHADER: " << fpath << "\n";
		shader.destroy();
		return -1;
	}

	if (gpath != nullptr && (loadText(gpath, text) < 0 ||
	                         !shader.attachString(Shader::Type::GEOMETRY, text))) {
		std::cerr << "GEOMETRY SHADER: " << gpath << "\n";
		shader.destroy();
		return -1;
	}

	if (!shader.link()) {
		std::cerr << "SHADER PROGRAM: Link error\n";
		shader.destroy();
		return -1;
	}

	if (Shaders.find(name) != Shaders.end()) {
		std::cerr << "SHADER '" << name << "' ALREADY LOADED\n";
		shader.destroy();
		return -1;
	}

	Shaders[name] = shader;
	return 0;
}

Shader const&
ResourceManager::GetShader(std::string name)
{
	return Shaders[name];
}

int
ResourceManager::LoadTexture(std::string name, const char *path, GLboolean alpha)
{
	(void)alpha;
	int width, height, channels;
	unsigned char *pixels = stbi_load(path, &width, &height, &channels, 4);
	if (pixels == nullptr)
	{
		std::cerr << "file '" << path << "' not found\n";
		return -1;
	}

	Texture2D texture;
	bool result = texture.create(width, height, pixels);
	stbi_image_free(pixels);
	if (!result)
	{
		std::cerr << "cannot generate the texture\n";
		return -1;
	}

	Textures[name] = texture;
	return 0;
}

Texture2D const&
ResourceManager::GetTexture(std::string name)
{
	return Textures[name];
}

void
ResourceManager::Clear()
{
	for (auto &s: Shaders) {
		s.second.destroy();
	}

	for (auto &t: Textures) {
		t.second.destroy();
	}
}

int
ResourceManager::loadText(const char *path, std::string& value)
{
	std::ifstream file(path);
	if (file.fail()) {
		std::cerr << "file '" << path << "' not found\n";
		return -1;
	}

	std::stringstream stream;
	stream << file.rdbuf();

	value = stream.str();
	return 0;
}
