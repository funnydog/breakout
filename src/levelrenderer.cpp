#include "glcheck.hpp"
#include "levelrenderer.hpp"

LevelRenderer::LevelRenderer(const Shader &s)
	: mShader(s)
{
}

void
LevelRenderer::draw(BatchRenderer &br, const GameLevel &level)
{
	static const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	static const glm::vec2 units[] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	static const glm::vec2 uvSize = {128.f/1024.f, 1.f};
	static const glm::vec2 uvPos = {128.f/1024.f, 0.f};

	mShader.use();
	// set the texture
	mVertices.clear();
	br.beginBatch();
	for (const auto &b : level.mBricks)
	{
		if (b.dead)
		{
			continue;
		}
		br.reserve(4, indices);
		for (auto unit : units)
		{
			mVertices.push_back(
				glm::vec4(level.mBrickSize * unit + b.position,
				          uvSize * unit + uvPos * b.type));
		}
	}
	br.endBatch();
	br.bindBuffers();
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mVertices.size() * sizeof(mVertices[0]),
	                     mVertices.data(),
	                     GL_STREAM_DRAW));
	br.drawBuffers();
}
