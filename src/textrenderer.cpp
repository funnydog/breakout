#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "font.hpp"
#include "glcheck.hpp"
#include "textrenderer.hpp"

TextRenderer::TextRenderer(const Shader &s)
	: mVertexOffset(0)
	, mIndexOffset(0)
	, mShader(s)
{
	// set the sampler2D to texture unit 0
	mShader.use();
	mShader.getUniform("text").setInteger(0);

	// create the EBO and VBO
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glGenBuffers(1, &mEBO));
	glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO));

	// save the state to a VAO
	glCheck(glGenVertexArrays(1, &mVAO));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));

        // unbind the VAO and the VBO
	glCheck(glBindVertexArray(0));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
	glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

TextRenderer::~TextRenderer()
{
	glCheck(glDeleteVertexArrays(1, &mVAO));
	glCheck(glDeleteBuffers(1, &mEBO));
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
	font.draw(*this, text, pos);
	glCheck(glBindVertexArray(0));
}

void
TextRenderer::draw() noexcept
{
	// upload the data
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER,
			     mVertices.size() * sizeof(mVertices[0]),
			     mVertices.data(),
			     GL_STREAM_DRAW));

	glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO));
	glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			     mIndices.size() * sizeof(mIndices[0]),
			     mIndices.data(),
			     GL_STREAM_DRAW));

	// draw the geometry
	saveCurrentBatch();
	for (const auto &batch : mBatches)
	{
		glCheck(glDrawElementsBaseVertex(
			        GL_TRIANGLES,
			        batch.mIndexCount,
			        GL_UNSIGNED_SHORT,
			        reinterpret_cast<GLvoid*>(batch.mIndexOffset),
			        batch.mVertexOffset));
	}
	mVertices.clear();
	mIndices.clear();
	mBatches.clear();
	mVertexOffset = mIndexOffset = 0;
}

void
TextRenderer::saveCurrentBatch()
{
	mBatches.emplace_back(
		mVertexOffset,
		mIndexOffset,
		mIndices.size()-mIndexOffset);
	mVertexOffset = mVertices.size();
	mIndexOffset = mIndices.size();
}

std::span<glm::vec4>
TextRenderer::reserve(unsigned vcount, std::span<const std::uint16_t> indices)
{
	auto size = mVertices.size();
	auto base = size - mVertexOffset;
	if (base + vcount > UINT16_MAX)
	{
		saveCurrentBatch();
		base = 0;
	}
	for (auto i : indices)
	{
		mIndices.push_back(base + i);
	}
	mVertices.resize(size + vcount);
	return std::span(mVertices.begin()+size, vcount);
}
