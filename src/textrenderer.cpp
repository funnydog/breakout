#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "glcheck.hpp"
#include "textrenderer.hpp"

TextRenderer::TextRenderer(const Shader &s)
	: mShader(s)
{
	// set the sampler2D to texture unit 0
	mShader.use();
	mShader.getUniform("text").setInteger(0);

	// reserve a VBO for the vertices
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW));

	// save the state to a VAO
	glCheck(glGenVertexArrays(1, &mVAO));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));

        // unbind the VAO and the VBO
	glCheck(glBindVertexArray(0));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

TextRenderer::~TextRenderer()
{
	glCheck(glDeleteVertexArrays(1, &mVAO));
	glCheck(glDeleteBuffers(1, &mVBO));
}

void
TextRenderer::draw(const std::string &text, glm::vec2 pos, Font &font, glm::vec3 color)
{
	mShader.use();
	mShader.getUniform("textColor").setVector3f(color);
	glCheck(glActiveTexture(GL_TEXTURE0));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	font.draw(text, pos);
	glCheck(glBindVertexArray(0));
}
