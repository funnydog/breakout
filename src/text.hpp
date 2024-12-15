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

	Shader TextShader;

	explicit TextRenderer(Shader const& s);

	void Load(std::string font, GLuint fontSize);

	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color = glm::vec3(1.0f));
private:
	GLuint VAO, VBO;
};
