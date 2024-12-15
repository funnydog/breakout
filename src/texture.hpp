#ifndef TEXTURE_H
#define TEXTURE_H

#include <filesystem>
#include <GL/glew.h>

class Texture2D
{
public:
	Texture2D();

	bool loadFromFile(const std::filesystem::path &path);
	bool create(unsigned width, unsigned height,
	            const void *pixels=nullptr,
	            bool repeat=false, bool smooth=false) noexcept;

	void destroy() noexcept;
	void bind() const noexcept;

	GLuint getHandle() const noexcept { return glHandle; }

private:
	GLuint glHandle;
};

#endif
