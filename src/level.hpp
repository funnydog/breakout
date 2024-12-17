#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "batchrenderer.hpp"
#include "resources.hpp"
#include "resourceholder.hpp"
#include "texture.hpp"

struct Brick
{
	glm::vec2 position;
	float type;
	bool solid;
	bool dead;
};

class GameLevel
{
public:
	bool load(const char *path, unsigned width, unsigned height);
	void draw(BatchRenderer &renderer);
	bool isCompleted();

	std::vector<Brick> mBricks;
	glm::vec2 mBrickSize;

private:
	std::vector<glm::vec4> mVertices;
};
