#include <iostream>

#include "font.hpp"
#include "glcheck.hpp"
#include "textrenderer.hpp"

TextRenderer::TextRenderer(const Shader &s)
	: mRenderer()
	, mShader(s)
{
	// set the sampler2D to texture unit 0
	mShader.use();
	mShader.getUniform("text").setInteger(0);
}

void
TextRenderer::draw(const std::string &text, glm::vec2 pos, Font &font, glm::vec3 color)
{
	mShader.use();
	mShader.getUniform("textColor").setVector3f(color);

	mVertices.clear();
	mRenderer.beginBatch();
	font.draw(*this, text, pos);
}

void
TextRenderer::draw() noexcept
{
	mRenderer.endBatch();

	mRenderer.bindBuffers();
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mVertices.size() * sizeof(mVertices[0]),
	                     mVertices.data(),
	                     GL_STREAM_DRAW));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));

	mRenderer.drawBuffers();
}

std::span<glm::vec4>
TextRenderer::reserve(unsigned vcount, std::span<const std::uint16_t> indices)
{
	mRenderer.reserve(vcount, indices);
	auto size = mVertices.size();
	mVertices.resize(size + vcount);
	return std::span(mVertices.begin()+size, vcount);
}
