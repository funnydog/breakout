#include <cassert>
#include <iostream>

#include "glcheck.hpp"
#include "texture.hpp"
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
Texture2D::update(const void *pixels)
{
	update(pixels, 0, 0, getWidth(), getHeight());
}

void
Texture2D::update(const void *pixels,
                  unsigned x, unsigned y,
                  unsigned w, unsigned h)
{
	assert(x + w <= getWidth() && "X target outside the texture");
	assert(y + h <= getHeight() && "Y target outside the texture");
	assert(pixels != nullptr && "empty bitmap");

	if (glHandle != -1U)
	{
		glCheck(glBindTexture(GL_TEXTURE_2D, glHandle));
		glCheck(glTexSubImage2D(GL_TEXTURE_2D,
		                        0,
		                        static_cast<GLint>(x),
		                        static_cast<GLint>(y),
		                        static_cast<GLsizei>(w),
		                        static_cast<GLsizei>(h),
		                        GL_RGBA,
		                        GL_UNSIGNED_BYTE,
		                        pixels));
		glCheck(glFlush());
	}
}

void
Texture2D::update(const Texture2D &other, unsigned x, unsigned y)
{
	auto dstWidth = getWidth();
	auto dstHeight = getHeight();
	auto srcWidth = other.getWidth();
	auto srcHeight = other.getHeight();
	assert(x + srcWidth <= dstWidth && "X target outside the texture");
	assert(y + srcHeight <= dstHeight && "Y target outside the texture");
	if (glHandle == -1U || other.glHandle == -1U)
	{
		return;
	}

	GLint oldReadFB, oldDrawFB;
	glCheck(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadFB));
	glCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDrawFB));

	GLuint readFB, drawFB;
	glCheck(glGenFramebuffers(1, &readFB));
	glCheck(glGenFramebuffers(1, &drawFB));

	glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFB));
	glCheck(glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
	                               GL_COLOR_ATTACHMENT0,
	                               GL_TEXTURE_2D,
	                               other.glHandle,
	                               0));

	glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFB));
	glCheck(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
	                               GL_COLOR_ATTACHMENT0,
	                               GL_TEXTURE_2D,
	                               glHandle,
	                               0));

	GLenum readStatus = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
	GLenum drawStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (readStatus != GL_FRAMEBUFFER_COMPLETE
	    || drawStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Texture2D::update"
		                         " - framebuffers incomplete.");
	}

	glCheck(glBlitFramebuffer(
		        0, 0, srcWidth, srcHeight,
		        x, y, x + srcWidth, x + srcHeight,
		        GL_COLOR_BUFFER_BIT,
		        GL_NEAREST));

	glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFB));
	glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDrawFB));

	glCheck(glDeleteFramebuffers(1, &readFB));
	glCheck(glDeleteFramebuffers(1, &drawFB));
}

void
Texture2D::destroy() noexcept
{
	if (glHandle != -1U)
	{
		glCheck(glDeleteTextures(1, &glHandle));
		glHandle = -1U;
	}
}

void
Texture2D::bind() const noexcept
{
	glCheck(glBindTexture(GL_TEXTURE_2D, glHandle));
}

void
Texture2D::bind(int textureUnit) const noexcept
{
	glCheck(glActiveTexture(GL_TEXTURE0 + textureUnit));
	glCheck(glBindTexture(GL_TEXTURE_2D, glHandle));
}

void
Texture2D::attachToFramebuffer(int level) const noexcept
{
	glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
	                               GL_TEXTURE_2D, glHandle, level));
}

unsigned
Texture2D::getWidth() const noexcept
{
	GLint width = 0;
	if (glHandle != -1U)
	{
		glCheck(glBindTexture(GL_TEXTURE_2D, glHandle));
		glCheck(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
		                                 GL_TEXTURE_WIDTH, &width));
	}
	return width;
}

unsigned
Texture2D::getHeight() const noexcept
{
	GLint height = 0;
	if (glHandle != -1U)
	{
		glCheck(glBindTexture(GL_TEXTURE_2D, glHandle));
		glCheck(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
		                                 GL_TEXTURE_HEIGHT, &height));
	}
	return height;
}
