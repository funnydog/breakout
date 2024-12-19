#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "glcheck.hpp"
#include "level.hpp"

bool
GameLevel::load(const char *path, const Texture2D &texture, unsigned levelWidth, unsigned levelHeight)
{
	mTexture = texture;
	mBricks.clear();

	std::ifstream input(path);
	if (input.fail())
	{
		std::cerr << "GameLevel::load() - failed to load \""
		          << path << "\".";
		return false;
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
	float unit_width = static_cast<float>(levelWidth / width);
	float unit_height = static_cast<float>(levelHeight / height);
	mBrickSize = glm::vec2(unit_width, unit_height);

	float offset = static_cast<float>((levelWidth % width) / 2);
	glm::vec2 pos(0.f);
	for (decltype(height) y = 0; y < height; ++y, pos.y += unit_height)
	{
		pos.x = offset;
		for (decltype(width) x = 0; x < width; ++x, pos.x += unit_width)
		{
			Brick b{pos, 0, false, false};

			switch (tileData[y][x])
			{
			case 1: b.solid = true;
			case 2:
			case 3:
			case 4:
			case 5: break;
			default: continue; // empty brick
			}
			b.type = tileData[y][x];
			mBricks.push_back(b);
		}
	}
	return true;
}

bool
GameLevel::isCompleted() const
{
	return std::all_of(mBricks.begin(), mBricks.end(), [](const auto &b) {
		return b.solid || b.dead;
	});
}

void
GameLevel::reset()
{
	for (auto &b : mBricks)
	{
		b.dead = false;
	}
}

glm::vec2
GameLevel::getBrickSize() const
{
	return mBrickSize;
}

const Texture2D&
GameLevel::getTexture() const
{
	return mTexture;
}
