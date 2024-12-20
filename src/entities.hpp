#pragma once

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
		SPEED,
		STICKY,
		PASSTHROUGH,
		PAD_INCREASE,
		CONFUSE,
		CHAOS,
	} type;
	Texture2D texture;
};
