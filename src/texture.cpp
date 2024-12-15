#include <iostream>

#include "glcheck.hpp"
#include "texture.h"
#include "stb_image.h"

Texture2D::Texture2D()
	: glHandle(0)
{
}

Texture2D::~Texture2D()
{
	// if (glHandle)
	// {
	// 	glCheck(glDeleteTextures(1, &glHandle));
	// }
}

Texture2D::Texture2D(Texture2D&& other) noexcept
	: glHandle(0)
{
	std::swap(glHandle, other.glHandle);
}

Texture2D&
Texture2D::operator=(Texture2D&& other) noexcept
{
	std::swap(glHandle, other.glHandle);
	return *this;
}

bool
Texture2D::loadFromFile(const std::filesystem::path &path)
{
	int width, height, channels;
	auto *pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
	if (pixels == nullptr)
	{
		std::cerr << "Texture2D::loadFromFile - Cannot load " << path.string()
		          << std::endl;
		return false;
	}

	bool result = create(width, height, pixels);
	stbi_image_free(pixels);
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

	if (!glHandle)
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
Texture2D::Destroy() noexcept
{
	glDeleteTextures(1, &this->glHandle);
	this->glHandle = 0;
}

void
Texture2D::GetSize(GLint &width, GLint &height) const noexcept
{
	glBindTexture(GL_TEXTURE_2D, this->glHandle);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLint
Texture2D::GetWidth() const noexcept
{
	GLint width, height;
	GetSize(width, height);
	return width;
}

GLint
Texture2D::GetHeight() const noexcept
{
	GLint width, height;
	GetSize(width, height);
	return height;
}

GLint
Texture2D::GetInternalFormat() const noexcept
{
	GLint format;
	glBindTexture(GL_TEXTURE_2D, this->glHandle);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
	glBindTexture(GL_TEXTURE_2D, 0);
	return format;
}

void
Texture2D::SetWrapS(GLint wraps) const noexcept
{
	glBindTexture(GL_TEXTURE_2D, this->glHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wraps);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void
Texture2D::SetWrapT(GLint wrapt) const noexcept
{
	glBindTexture(GL_TEXTURE_2D, this->glHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapt);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void
Texture2D::SetMinFilter(GLint minfilter) const noexcept
{
	glBindTexture(GL_TEXTURE_2D, this->glHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void
Texture2D::SetMagFilter(GLint magfilter) const noexcept
{
	glBindTexture(GL_TEXTURE_2D, this->glHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void
Texture2D::Bind() const noexcept
{
	glBindTexture(GL_TEXTURE_2D, this->glHandle);
}