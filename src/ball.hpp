#ifndef BALL_H
#define BALL_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "sprite.hpp"
#include "gameobject.hpp"

class BallObject : public GameObject
{
public:
	float Radius;
	bool Stuck;
	bool Sticky;
	bool PassThrough;

	BallObject(glm::vec2 pos, float radius, glm::vec2 vel, Texture2D sprite);

	glm::vec2 Move(float dt, GLuint window_width);

	void Reset(glm::vec2 pos, glm::vec2 vel);
};

#endif
