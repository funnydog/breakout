#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "batchrenderer.hpp"
#include "particle.hpp"
#include "shader.hpp"

class ParticleRenderer
{
public:
	explicit ParticleRenderer(const Shader &s);

	void draw(BatchRenderer &br, const ParticleGen &pg);

private:
	struct Vertex
	{
		glm::vec2 pos;
		glm::vec2 uv;
		glm::vec4 color;
	};

	Shader mShader;
	std::vector<Vertex> mVertices;
};
