#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "shader.hpp"

class Font;

class TextRenderer
{
public:
	explicit TextRenderer(const Shader &s);
	~TextRenderer();

	void draw(const std::string &text, glm::vec2 pos, Font &font, glm::vec3 color=glm::vec3(1.0f));

	std::span<glm::vec4> reserve(unsigned vcount,
	                             std::span<const std::uint16_t> indices);

	void draw() noexcept;

private:
	std::vector<glm::vec4> mVertices;
	std::vector<std::uint16_t> mIndices;
	unsigned mVertexOffset;
	unsigned mIndexOffset;

	Shader mShader;
	GLuint mVAO;
	GLuint mVBO;
	GLuint mEBO;
};
