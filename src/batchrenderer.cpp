#include <iostream>

#include "glcheck.hpp"
#include "batchrenderer.hpp"

BatchRenderer::BatchRenderer()
	: mVertexOffset(0)
	, mVertexCount(0)
	, mIndexOffset(0)
	, mIndexCount(0)
{
        // TODO: set the initial shader uniforms

	// create the EBO and VBO
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glGenBuffers(1, &mEBO));

	// create a VAO and save the relevant state
	glCheck(glGenVertexArrays(1, &mVAO));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	glCheck(glBindVertexArray(0));
}

BatchRenderer::~BatchRenderer()
{
	glCheck(glBindVertexArray(0));
	glCheck(glDeleteVertexArrays(1, &mVAO));
	glCheck(glDeleteBuffers(1, &mEBO));
	glCheck(glDeleteBuffers(1, &mVBO));
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
	// reset the data
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
	glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO));
}

void
BatchRenderer::drawBuffers() const
{
	// upload the indices
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
	glCheck(glBindVertexArray(0));
}
