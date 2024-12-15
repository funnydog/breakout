#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "glcheck.hpp"
#include "sprite.hpp"

SpriteRenderer::SpriteRenderer(Shader const& shader) :
	shader(shader)
{
	GLuint vbo;
	static const GLfloat vertices[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
	};

	glCheck(glGenVertexArrays(1, &quadVAO));
	glCheck(glBindVertexArray(quadVAO));

	glCheck(glGenBuffers(1, &vbo));

	glCheck(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	glCheck(glEnableVertexAttribArray(0));

	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
	glCheck(glBindVertexArray(0));
}

SpriteRenderer::~SpriteRenderer()
{
	glCheck(glDeleteVertexArrays(1, &quadVAO));
}

void
SpriteRenderer::DrawSprite(Texture2D &texture, glm::vec2 position, glm::vec2 size, GLfloat rotate, glm::vec3 color)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0f));
	model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
	model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));

	shader.use();
	shader.getUniform("model").setMatrix4(model);
	shader.getUniform("spriteColor").setVector3f(color);

	texture.bind(0);
	glCheck(glBindVertexArray(quadVAO));
	glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));
	glCheck(glBindVertexArray(0));
}
