#ifndef POWERUP_H
#define POWERUP_H

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "gameobject.h"

const glm::vec2 POWERUP_SIZE(60, 20);
const glm::vec2 POWERUP_VELOCITY(0.0f, 150.0f);

class PowerUP : public GameObject
{
public:
	enum Type {
		SPEED,
		STICKY,
		PASSTHROUGH,
		PAD_INCREASE,
		CONFUSE,
		CHAOS,
	} Type;
	float Duration;
	bool Activated;

	PowerUP(enum Type type, glm::vec3 color, float duration, glm::vec2 pos, Texture2D tex) :
		GameObject(pos, POWERUP_SIZE, tex, color, POWERUP_VELOCITY),
		Type(type),
		Duration(duration),
		Activated(false)
	{}
};

#endif
