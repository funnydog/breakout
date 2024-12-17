#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "glcheck.hpp"
#include "spriterenderer.hpp"

SpriteRenderer::SpriteRenderer(Shader const& shader) :
	mShader(shader)
{
	// set the sampler2D to the texture unit 0
	mShader.use();
	mShader.getUniform("image").setInteger(0);

	// upload the vertex data to the VBO
	static const GLfloat vertices[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
	};
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

	// state for the VAO
	glCheck(glGenVertexArrays(1, &mVAO));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));

	// unbind the VAO and the VBO
	glCheck(glBindVertexArray(0));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

SpriteRenderer::~SpriteRenderer()
{
	glCheck(glDeleteVertexArrays(1, &mVAO));
	glCheck(glDeleteBuffers(1, &mVBO));
}

void
SpriteRenderer::draw(Texture2D &texture, glm::vec2 position, glm::vec2 size, glm::vec3 color)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));

	mShader.use();
	mShader.getUniform("model").setMatrix4(model);
	mShader.getUniform("spriteColor").setVector3f(color);

	texture.bind(0);
	glCheck(glBindVertexArray(mVAO));
	glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));
	glCheck(glBindVertexArray(0));
}
