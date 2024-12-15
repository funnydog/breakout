#ifndef GAMELEVEL_H
#define GAMELEVEL_H

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "gameobject.hpp"
#include "sprite.hpp"
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

#endif
