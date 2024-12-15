#include "level.h"

#include <fstream>
#include <sstream>

int
GameLevel::Load(TextureHolder& textures, const char *path, GLuint levelWidth, GLuint levelHeight)
{
	Bricks.clear();

	std::ifstream input(path);
	if (input.fail())
	{
		return -1;
	}

	std::vector<std::vector<unsigned>> tileData;
	std::string line;

	while (std::getline(input, line))
	{
		std::istringstream ss(line);
		std::vector<unsigned> row;

		unsigned tileCode;
		while (ss >> tileCode)
		{
			row.push_back(tileCode);
		}

		tileData.push_back(row);
	}

	auto height = tileData.size();
	auto width = tileData[0].size();
	GLfloat unit_width = static_cast<GLfloat>(levelWidth) / width;
	GLfloat unit_height = static_cast<GLfloat>(levelHeight)  / height;

	glm::vec2 pos(0.0f);
	glm::vec2 size(unit_width, unit_height);
	for (decltype(height) y = 0; y < height; ++y)
	{
		for (decltype(width) x = 0; x < width; ++x)
		{
			if (tileData[y][x] == 1)
			{
				// solid block
				GameObject obj(
					pos, size,
					textures.get(TextureID::BlockSolid),
					glm::vec3(0.8f, 0.8f, 0.7f));
				obj.IsSolid = true;
				Bricks.push_back(obj);
			}
			else if (tileData[y][x] > 1)
			{
				// normal block
				glm::vec3 color;
				switch (tileData[y][x])
				{
				case 2: color = glm::vec3(0.2f, 0.6f, 1.0f); break;
				case 3: color = glm::vec3(0.0f, 0.7f, 0.0f); break;
				case 4: color = glm::vec3(0.8f, 0.8f, 0.4f); break;
				case 5: color = glm::vec3(1.0f, 0.5f, 0.0f); break;
				default: color = glm::vec3(1.0f); break;
				}
				GameObject obj(
					pos, size,
					textures.get(TextureID::Block),
					color);
				Bricks.push_back(obj);
			}
			pos.x += unit_width;
		}
		pos.x = 0;
		pos.y += unit_height;
	}
	return 0;
}

void
GameLevel::Draw(SpriteRenderer &renderer)
{
	for (auto &obj : Bricks) {
		if (!obj.Destroyed)
			obj.Draw(renderer);
	}
}

bool
GameLevel::IsCompleted()
{
	for (const auto &obj : Bricks)
	{
		if (!obj.IsSolid && !obj.Destroyed)
		{
			return false;
		}
	}
	return true;
}
