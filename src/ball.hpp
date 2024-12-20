#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "gameobject.hpp"

struct BallObject : public GameObject
{
	bool Stuck;
};
