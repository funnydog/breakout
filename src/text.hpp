#pragma once

#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "shader.hpp"

struct Character
{
	GLuint TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	GLuint Advance;
};

class TextRenderer
{
public:
	std::map<GLchar, Character> Characters;

	explicit TextRenderer(Shader const& s);

	void load(std::string font, GLuint fontSize);

	void draw(std::string text, glm::vec2 pos, float scale, glm::vec3 color = glm::vec3(1.0f));

private:
	Shader TextShader;
	GLuint VAO, VBO;
};
