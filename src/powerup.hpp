#pragma once

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "gameobject.hpp"

struct PowerUP : public GameObject
{
	enum Type {
		SPEED,
		STICKY,
		PASSTHROUGH,
		PAD_INCREASE,
		CONFUSE,
		CHAOS,
	} Type;
};
