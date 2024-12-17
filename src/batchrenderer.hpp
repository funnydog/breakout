#pragma once

#include <span>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

class BatchRenderer
{
public:
	BatchRenderer();
	~BatchRenderer();

	void reserve(unsigned vcount, std::span<const std::uint16_t> indices);

	void beginBatch();
	void endBatch();

	void bindBuffers() const;
	void drawBuffers() const;

private:
	struct Batch
	{
		unsigned vertexOffset;
		unsigned indexOffset;
		unsigned indexCount;
	};

	void saveBatch();

	std::vector<Batch> mBatches;
	std::vector<std::uint16_t> mIndices;
	unsigned mVertexOffset;
	unsigned mVertexCount;
	unsigned mIndexOffset;
	unsigned mIndexCount;

	GLuint mVAO;
	GLuint mVBO;
	GLuint mEBO;
};
