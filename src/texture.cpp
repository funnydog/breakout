#include <iostream>

#include "glcheck.hpp"
#include "texture.h"
#include "stb_image.h"

Texture2D::Texture2D()
	: glHandle(-1U)
{
}

bool
Texture2D::loadFromFile(const std::filesystem::path &path)
{
	int width, height, channels;
	auto *pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
	if (pixels == nullptr)
	{
		std::cerr << "Texture2D::loadFromFile() - Cannot load " << path.string()
		          << std::endl;
		return false;
	}

	bool result = create(width, height, pixels);
	stbi_image_free(pixels);
	if (!result)
	{
		std::cerr << "Texture2D::loadFromFile() - unable to generate the texture "
		          << path.string() << std::endl;
	}
	return result;
}

bool
Texture2D::create(unsigned width, unsigned height, const void *pixels, bool repeat, bool smooth) noexcept
{
	if (!width || !height)
	{
		std::cerr << "Texture2D::create() - invalid size ("
		          << width << ", " << height << ")\n";
		return false;
	}

	if (glHandle == -1U)
	{
		glCheck(glGenTextures(1, &glHandle));
	}

	glCheck(glBindTexture(GL_TEXTURE_2D, glHandle));
	glCheck(glTexImage2D(
		        GL_TEXTURE_2D,
		        0,
		        GL_RGBA,
		        static_cast<GLsizei>(width),
		        static_cast<GLsizei>(height),
		        0,
		        GL_RGBA,
		        GL_UNSIGNED_BYTE,
		        pixels));

	GLint param = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param));

	param = smooth ? GL_LINEAR : GL_NEAREST;
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param));
	return true;
}

void
Texture2D::destroy() noexcept
{
	if (glHandle != -1U)
	{
		glDeleteTextures(1, &glHandle);
		glHandle = -1U;
	}
}

void
Texture2D::bind() const noexcept
{
	glBindTexture(GL_TEXTURE_2D, glHandle);
}
