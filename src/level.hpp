#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "gameobject.hpp"
#include "spriterenderer.hpp"
#include "resources.hpp"
#include "resourceholder.hpp"

class GameLevel
{
public:
	std::vector<GameObject> Bricks;

	int Load(TextureHolder& textures, const char *path, GLuint levelWidth, GLuint levelHeight);
	void Draw(SpriteRenderer &renderer);
	bool IsCompleted();
};
