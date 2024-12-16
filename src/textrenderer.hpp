#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "font.hpp"
#include "texture.hpp"
#include "shader.hpp"

class TextRenderer
{
public:
	explicit TextRenderer(const Shader &s);
	~TextRenderer();

	void draw(const std::string &text, glm::vec2 pos, Font &font, glm::vec3 color=glm::vec3(1.0f));
private:
	Shader mShader;
	GLuint mVAO;
	GLuint mVBO;
};
