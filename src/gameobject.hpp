#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"

class GameObject
{
public:
	glm::vec2 Position;
	glm::vec2 Size;
	glm::vec2 Velocity;
	glm::vec3 Color;
	GLboolean IsSolid;
	GLboolean Destroyed;

	Texture2D Sprite;

	GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color = glm::vec3(1.0f), glm::vec2 vel = glm::vec2(0.0f));
};
