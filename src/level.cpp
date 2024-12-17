#include "level.hpp"

#include <algorithm>
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
	for (decltype(height) y = 0; y < height; ++y, pos.y += unit_height)
	{
		pos.x = 0.f;
		for (decltype(width) x = 0; x < width; ++x, pos.x += unit_width)
		{
			glm::vec3 color;
			bool isSolid = false;
			TextureID textureID = TextureID::Block;

			switch (tileData[y][x])
			{
			case 0: continue; // no brick, skip
			case 1: color = glm::vec3(0.8f, 0.8f, 0.7f);
				isSolid = true;
				textureID = TextureID::BlockSolid;
				break;
			case 2: color = glm::vec3(0.2f, 0.6f, 1.0f); break;
			case 3: color = glm::vec3(0.0f, 0.7f, 0.0f); break;
			case 4: color = glm::vec3(0.8f, 0.8f, 0.4f); break;
			case 5: color = glm::vec3(1.0f, 0.5f, 0.0f); break;
			default: color = glm::vec3(1.0f); break;
			}

			// insert the brick
			auto &brick = Bricks.emplace_back(
				pos, size, textures.get(textureID), color);
			brick.IsSolid = isSolid;
		}
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
	return std::all_of(Bricks.begin(), Bricks.end(), [](const auto &b) {
		return b.IsSolid || b.Destroyed;
	});
}
