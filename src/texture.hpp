#pragma once

#include <filesystem>
#include <GL/glew.h>

class Texture2D
{
public:
	Texture2D();

	bool loadFromFile(const std::filesystem::path &path);
	bool create(unsigned width, unsigned height,
	            const void *pixels=nullptr,
	            bool repeat=false, bool smooth=true) noexcept;

	void update(const void *pixels);
	void update(const void *pixels,
	            unsigned x, unsigned y,
	            unsigned w, unsigned h);
	void update(const Texture2D &other,
	            unsigned x=0, unsigned y=0);

	void destroy() noexcept;

	void bind() const noexcept;
	void bind(int textureUnit) const noexcept;
	void attachToFramebuffer(int level) const noexcept;

	unsigned getWidth() const noexcept;
	unsigned getHeight() const noexcept;

private:
	GLuint glHandle;
};
