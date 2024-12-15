#ifndef RESOURCE_H
#define RESOURCE_H

#include <map>
#include <string>

#include <GL/glew.h>

#include "texture.h"
#include "shader.h"

class ResourceManager
{
public:
	static int LoadShader(std::string name, const char *vPath, const char *tPath, const char *gPath);
	static Shader const& GetShader(std::string name);

	static int LoadTexture(std::string name, const char *path, GLboolean alpha);
	static Texture2D const& GetTexture(std::string name);

	static void Clear();

private:
	ResourceManager() {}

	static int loadText(const char *path, std::string &str);

	static std::map<std::string, Shader> Shaders;
	static std::map<std::string, Texture2D> Textures;
};

#endif
