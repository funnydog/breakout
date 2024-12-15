#ifndef GAMELEVEL_H
#define GAMELEVEL_H

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "gameobject.h"
#include "sprite.h"
#include "resource.h"

class GameLevel
{
public:
	std::vector<GameObject> Bricks;

	int Load(const char *path, GLuint levelWidth, GLuint levelHeight);
	void Draw(SpriteRenderer &renderer);
	bool IsCompleted();
};

#endif
