#include <iostream>

#include "texture.h"

void
Texture2D::Generate(GLuint width, GLuint height, GLint ifmt, GLint dfmt, void *data) noexcept
{
	glGenTextures(1, &this->glHandle);
	glBindTexture(GL_TEXTURE_2D, this->glHandle);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		ifmt,
		width,
		height,
		0,
		dfmt,
		GL_UNSIGNED_BYTE,
		data);

	// default texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
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
