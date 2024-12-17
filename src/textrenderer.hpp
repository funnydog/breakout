#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "batchrenderer.hpp"
#include "shader.hpp"

class Font;

class TextRenderer
{
public:
	explicit TextRenderer(const Shader &s);

	void draw(const std::string &text, glm::vec2 pos,
	          Font &font, glm::vec3 color=glm::vec3(1.0f));

private:
	std::vector<glm::vec4> mVertices;
	BatchRenderer mRenderer;
	Shader mShader;
};
