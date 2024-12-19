#include "ball.hpp"

BallObject::BallObject(glm::vec2 pos, float radius, glm::vec2 vel, Texture2D sprite) :
	GameObject(pos, glm::vec2(radius * 2, radius * 2), sprite, glm::vec3(1.0f), vel),
	Stuck(true), Sticky(false), PassThrough(false)
{
}

glm::vec2
BallObject::Move(float dt, GLuint window_width)
{
	if (Stuck)
	{
		return Position;
	}

	Position += Velocity * dt;

	if (Position.x <= 0.0f)
	{
		Velocity.x = -Velocity.x;
		Position.x = 0.0f;
	}
	else if (Position.x + Size.x >= window_width)
	{
		Velocity.x = -Velocity.x;
		Position.x = window_width - Size.x;
	}

	if (Position.y <= 0.0f)
	{
		Velocity.y = -Velocity.y;
		Position.y = 0.0f;
	}

	return Position;
}

void
BallObject::Reset(glm::vec2 pos, glm::vec2 vel)
{
	Position = pos;
	Velocity = vel;
	Color = glm::vec3(1.0f);
	Stuck = true;
	Sticky = false;
	PassThrough = false;
}
