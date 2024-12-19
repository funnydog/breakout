#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "renderer.hpp"
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
	bool load(const char *path, const Texture2D &texture, unsigned width, unsigned height);

	bool isCompleted() const;
	void reset();

	glm::vec2 getBrickSize() const;
	const Texture2D& getTexture() const;

public:
	std::vector<Brick> mBricks;

private:

	glm::vec2 mBrickSize;
	Texture2D mTexture;
};
