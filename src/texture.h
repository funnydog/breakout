#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>

class Texture2D
{
public:
	Texture2D() : glHandle(0) {}

	void Generate(GLuint width, GLuint height, GLint ifmt, GLint dfmt, void *data) noexcept;
	void Destroy() noexcept;

	void GetSize(GLint &width, GLint &height) const noexcept;
	GLint GetWidth() const noexcept;
	GLint GetHeight() const noexcept;

	GLint GetInternalFormat() const noexcept;

	void SetWrapS(GLint wraps) const noexcept;
	void SetWrapT(GLint wrapt) const noexcept;
	void SetMinFilter(GLint minfilter) const noexcept;
	void SetMagFilter(GLint magfilter) const noexcept;

	void Bind() const noexcept;

	GLuint GetHandle() const noexcept { return glHandle; }

private:
	GLuint glHandle;
};

#endif
