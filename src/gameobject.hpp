#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"

struct GameObject
{
	glm::vec2 Position;
	glm::vec2 Size;
	glm::vec2 Velocity;
	glm::vec3 Color;
	bool Destroyed;
	Texture2D Sprite;
};
