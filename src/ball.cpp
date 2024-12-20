#include "ball.hpp"

BallObject::BallObject(glm::vec2 pos, float radius, glm::vec2 vel, Texture2D sprite) :
	GameObject(pos, glm::vec2(radius * 2, radius * 2), sprite, glm::vec3(1.0f), vel),
	Stuck(true)
{
}
