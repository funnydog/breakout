#include <iostream>

#include "font.hpp"
#include "glcheck.hpp"
#include "batchrenderer.hpp"

BatchRenderer::BatchRenderer(const Shader &textShader)
	: mVertexOffset(0)
	, mVertexCount(0)
	, mIndexOffset(0)
	, mIndexCount(0)
	, mTextShader(textShader)
{
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glGenBuffers(1, &mEBO));
	glCheck(glGenVertexArrays(1, &mVAO));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glEnableVertexAttribArray(0));
}

BatchRenderer::~BatchRenderer()
{
	glCheck(glBindVertexArray(0));
	glCheck(glDeleteVertexArrays(1, &mVAO));
	glCheck(glDeleteBuffers(1, &mEBO));
	glCheck(glDeleteBuffers(1, &mVBO));
}

void
BatchRenderer::draw(const std::string &text, glm::vec2 pos, Font &font, glm::vec3 color)
{
	if (text.empty())
	{
		return;
	}

	// ensure the needed glyphs are rendered
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
	for (auto codepoint: cv.from_bytes(text))
	{
		font.getGlyph(codepoint);
	}

	mTextShader.use();
	mTextShader.getUniform("textColor").setVector3f(color);
	font.getTexture().bind(0);

	static const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	static const glm::vec2 units[] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};

	mSimpleVertices.clear();
	beginBatch();
	pos.y += font.getLineHeight();
	for (auto codepoint : cv.from_bytes(text))
	{
		const auto &g = font.getGlyph(codepoint);
		pos.x += g.bearing.x;
		pos.y -= g.bearing.y;
		reserve(4, indices);
		for (auto unit : units)
		{
			mSimpleVertices.push_back(
				glm::vec4(g.size*unit+pos,
				          g.uvSize*unit+g.uvPos));
		}
		pos.x += g.advance - g.bearing.x;
		pos.y += g.bearing.y;
	}
	endBatch();

	bindBuffers();
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mSimpleVertices.size() * sizeof(mSimpleVertices[0]),
	                     mSimpleVertices.data(),
	                     GL_STREAM_DRAW));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	drawBuffers();
}

void
BatchRenderer::saveBatch()
{
	mBatches.emplace_back(mVertexOffset, mIndexOffset, mIndexCount-mIndexOffset);
	mVertexOffset = mVertexCount;
	mIndexOffset = mIndexCount;
}

void
BatchRenderer::reserve(unsigned vcount, std::span<const std::uint16_t> indices)
{
	auto base = mVertexCount - mVertexOffset;
	if (base + vcount > UINT16_MAX)
	{
		saveBatch();
		base = 0;
	}
	mVertexCount += vcount;
	for (auto i : indices)
	{
		mIndices.push_back(base + i);
	}
	mIndexCount += indices.size();
}

void
BatchRenderer::beginBatch()
{
	mIndices.clear();
	mBatches.clear();
	mVertexOffset = mVertexCount = 0;
	mIndexOffset = mIndexCount = 0;
}

void
BatchRenderer::endBatch()
{
	saveBatch();
}

void
BatchRenderer::bindBuffers() const
{
	glCheck(glBindVertexArray(mVAO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
}

void
BatchRenderer::drawBuffers() const
{
	// upload the indices
	glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO));
	glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			     mIndices.size() * sizeof(mIndices[0]),
			     mIndices.data(),
			     GL_STREAM_DRAW));

	// draw all the batches
	for (const auto &batch : mBatches)
	{
		glCheck(glDrawElementsBaseVertex(
			        GL_TRIANGLES,
			        batch.indexCount,
			        GL_UNSIGNED_SHORT,
			        reinterpret_cast<GLvoid*>(batch.indexOffset),
			        batch.vertexOffset));
	}
}
