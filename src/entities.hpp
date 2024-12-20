#pragma once

#include <vector>

#include <glm/glm.hpp>
#include "texture.hpp"

struct Paddle
{
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec2 vel;
	glm::vec3 color;
	bool dead;
	Texture2D texture;
};

struct Ball
{
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec2 vel;
	glm::vec3 color;
	bool stuck;
	Texture2D texture;
};

struct PowerUP
{
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec2 vel;
	glm::vec3 color;
	bool dead;
	enum Type
	{
		Speed,
		Sticky,
		PassThrough,
		PadIncrease,
		Confuse,
		Chaos,
	} type;
	Texture2D texture;
};

struct Block
{
	glm::vec2 position;
	unsigned type;
	bool solid;
	bool dead;
};

struct Level
{
	std::vector<Block> blocks;
	glm::vec2 blockSize;
	Texture2D texture;
};
