#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "glcheck.hpp"
#include "level.hpp"

bool
GameLevel::load(const char *path, unsigned levelWidth, unsigned levelHeight)
{
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
	GLfloat unit_width = static_cast<GLfloat>(levelWidth) / width;
	GLfloat unit_height = static_cast<GLfloat>(levelHeight)  / height;
	mBrickSize = glm::vec2(unit_width, unit_height);

	glm::vec2 pos(0.0f);
	for (decltype(height) y = 0; y < height; ++y, pos.y += unit_height)
	{
		pos.x = 0.f;
		for (decltype(width) x = 0; x < width; ++x, pos.x += unit_width)
		{
			Brick b{pos, 0, false, false};

			switch (tileData[y][x])
			{
			case 0: b.dead = true; break; // no brick
			case 1: b.solid = true; break;
			case 2: break;
			case 3: break;
			case 4: break;
			case 5: break;
			default: continue; // unknown brick
			}
			b.type = tileData[y][x];
			mBricks.push_back(b);
		}
	}
	return true;
}

void
GameLevel::draw(BatchRenderer &br)
{
	static const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	static const glm::vec2 unit[] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	static const glm::vec2 uvSize = {128.f/1024.f, 1.f};
	static const glm::vec2 uvPos = {128.f/1024.f, 0.f};

	mVertices.clear();
	br.beginBatch();
	for (const auto &b : mBricks)
	{
		if (!b.dead)
		{
			br.reserve(4, indices);
			for (int i = 0; i < 4; i++)
			{
				mVertices.push_back(
					glm::vec4(mBrickSize*unit[i] + b.position,
					          uvSize*unit[i] + uvPos * b.type));
			}
		}
	}
	br.endBatch();

	br.bindBuffers();

	// upload the data
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mVertices.size() * sizeof(mVertices[0]),
	                     mVertices.data(),
	                     GL_STREAM_DRAW));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));

	br.drawBuffers();
}

bool
GameLevel::isCompleted()
{
	return std::all_of(mBricks.begin(), mBricks.end(), [](const auto &b) {
		return b.solid || b.dead;
	});
}
