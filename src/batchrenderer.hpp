#pragma once

#include <span>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.hpp"

class Font;

class BatchRenderer
{
public:
	explicit BatchRenderer(const Shader &textShader);
	~BatchRenderer();

	void draw(const std::string &text, glm::vec2 pos,
	          Font &font, glm::vec3 color = glm::vec3(1.0f));

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
	std::vector<glm::vec4> mSimpleVertices;
	unsigned mVertexOffset;
	unsigned mVertexCount;
	unsigned mIndexOffset;
	unsigned mIndexCount;

	Shader mTextShader;

	GLuint mVAO;
	GLuint mVBO;
	GLuint mEBO;
};
