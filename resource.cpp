#include <iostream>
#include <sstream>
#include <fstream>

#include <SOIL/SOIL.h>

#include "resource.h"

std::map<std::string, Shader> ResourceManager::Shaders;
std::map<std::string, Texture2D> ResourceManager::Textures;

int
ResourceManager::LoadShader(std::string name, const char *vpath, const char *fpath, const char *gpath)
{
	Shader shader;
	if (shader.Create() < 0) {
		std::cerr << "SHADER PROGRAM:\n";
		return -1;
	}

	std::string text;
	if (loadText(vpath, text) < 0
	    || shader.Attach(Shader::Type::VERTEX, text.c_str()) < 0) {
		std::cerr << "VERTEX SHADER: " << vpath << "\n";
		shader.Destroy();
		return -1;
	}

	if (loadText(fpath, text) < 0
	    || shader.Attach(Shader::Type::FRAGMENT, text.c_str()) < 0) {
		std::cerr << "FRAGMENT SHADER: " << fpath << "\n";
		shader.Destroy();
		return -1;
	}

	if (gpath != nullptr && (loadText(gpath, text) < 0 ||
				 shader.Attach(Shader::Type::GEOMETRY, text.c_str()) < 0)) {
		std::cerr << "GEOMETRY SHADER: " << gpath << "\n";
		shader.Destroy();
		return -1;
	}

	if (shader.Link() < 0) {
		std::cerr << "SHADER PROGRAM: Link error\n";
		shader.Destroy();
		return -1;
	}

	if (Shaders.find(name) != Shaders.end()) {
		std::cerr << "SHADER '" << name << "' ALREADY LOADED\n";
		shader.Destroy();
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
	int width, height;
	unsigned char *image = SOIL_load_image(
		path,
		&width,
		&height,
		0,
		alpha ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);
	if (image == nullptr) {
		std::cerr << "file '" << path << "' not found\n";
		return -1;
	}

	GLint fmt = alpha ? GL_RGBA : GL_RGB;
	Texture2D texture;
	texture.Generate(width, height, fmt, fmt, image);
	SOIL_free_image_data(image);

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
		s.second.Destroy();
	}

	for (auto &t: Textures) {
		t.second.Destroy();
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
