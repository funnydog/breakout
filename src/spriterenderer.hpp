#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "shader.hpp"

class SpriteRenderer
{
public:
	explicit SpriteRenderer(Shader const& shader);
	~SpriteRenderer();

	void draw(Texture2D &texture, glm::vec2 position, glm::vec2 size = glm::vec2(10,10), glm::vec3 color = glm::vec3(1.0f));

private:
	Shader mShader;
	GLuint mVAO;
	GLuint mVBO;
};
