#include "gameobject.hpp"

GameObject::GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color, glm::vec2 vel) :
	Position(pos), Size(size), Velocity(vel), Color(color), Rotation(0.0f),
	IsSolid(false), Destroyed(false), Sprite(sprite)
{
}

GameObject::~GameObject()
{
}

void
GameObject::Draw(SpriteRenderer &renderer)
{
	renderer.draw(Sprite, Position, Size, Rotation, Color);
}
