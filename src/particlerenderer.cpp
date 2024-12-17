#include <cstddef>

#include "glcheck.hpp"
#include "particlerenderer.hpp"

ParticleRenderer::ParticleRenderer(const Shader &s)
	: mShader(s)
{
}

void
ParticleRenderer::draw(BatchRenderer &br, const ParticleGen &pg)
{
	static const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	static const glm::vec2 units[] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};

	mShader.use();
	mVertices.clear();
	auto size = pg.getParticleSize();
	br.beginBatch();
	for (const auto &p : pg.getParticles())
	{
		if (p.life > 0.f)
		{
			br.reserve(4, indices);
			for (auto unit : units)
			{
				Vertex v;
				v.pos = size * unit + p.position;
				v.uv = unit;
				v.color = p.color;
				mVertices.push_back(v);
			}
		}
	}
	br.endBatch();
	br.bindBuffers();
	glEnableVertexAttribArray(1);
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));
	glCheck(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                              reinterpret_cast<GLvoid*>(offsetof(Vertex, color))));
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mVertices.size() * sizeof(mVertices[0]),
	                     mVertices.data(),
	                     GL_STREAM_DRAW));
	pg.getTexture().bind(0);

	// set an additive blending for the glow effect
	glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
	br.drawBuffers();
	// restore the previous blend function
	glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}
