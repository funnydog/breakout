#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "gameobject.hpp"

class BallObject : public GameObject
{
public:
	bool Stuck;

	BallObject(glm::vec2 pos, float radius, glm::vec2 vel, Texture2D sprite);

	glm::vec2 Move(float dt, GLuint window_width);
};
