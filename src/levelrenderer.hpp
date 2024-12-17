#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "batchrenderer.hpp"
#include "level.hpp"
#include "shader.hpp"

class LevelRenderer
{
public:
	explicit LevelRenderer(const Shader &s);
	void draw(BatchRenderer &br, const GameLevel &level);
private:
	std::vector<glm::vec4> mVertices;
	Shader mShader;
};
