#pragma once

#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "shader.hpp"

class TextRenderer
{
public:
	explicit TextRenderer(const Shader &s);
	~TextRenderer();

	bool load(const std::string &font, GLuint fontSize) noexcept;
	void draw(const std::string &text, glm::vec2 pos, float scale, glm::vec3 color = glm::vec3(1.0f));

private:
	struct Character
	{
		GLuint TextureID;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		GLuint Advance;
	};
	std::map<GLchar, Character> mCharacters;
	Shader mShader;
	GLuint mVAO;
	GLuint mVBO;
};
