#include "gameobject.hpp"

GameObject::GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color, glm::vec2 vel)
	: Position(pos)
	, Size(size)
	, Velocity(vel)
	, Color(color)
	, Destroyed(false)
	, Sprite(sprite)
{
}
