#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "batchrenderer.hpp"
#include "texture.hpp"

struct Brick
{
	glm::vec2 position;
	float type;
	bool solid;
	bool dead;
};

struct GameLevel
{
	bool load(const char *path, unsigned width, unsigned height);
	std::vector<Brick> mBricks;
	glm::vec2 mBrickSize;

	void draw(BatchRenderer &renderer);
	bool isCompleted() const;
};
