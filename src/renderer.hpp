#pragma once

#include <span>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.hpp"
#include "entities.hpp"

class Font;
class ParticleGen;
class Texture2D;

class Renderer
{
public:
	Renderer(const Shader &textShader,
	              const Shader &levelShader,
	              const Shader &particleShader,
	              const Shader &spriteShader);
	~Renderer();

	void clear(glm::vec4 color) const;
	void draw(const std::string &text, glm::vec2 pos,
	          Font &font, glm::vec3 color = glm::vec3(1.0f));

	void draw(const Paddle &player);
	void draw(const Ball &ball);
	void draw(const PowerUP &pow);
	void draw(const Level &level);
	void draw(const ParticleGen &pg);

	void draw(Texture2D texture, glm::vec2 pos, glm::vec2 size,
	          glm::vec3 color = glm::vec3(1.0f));
private:

	void reserve(unsigned vcount, std::span<const std::uint16_t> indices);
	void beginBatch();
	void endBatch();

	void bindBuffers() const;
	void drawBuffers() const;

private:
	struct Batch
	{
		unsigned vertexOffset;
		unsigned indexOffset;
		unsigned indexCount;
	};

	struct SimpleVertex
	{
		glm::vec2 pos;
		glm::vec2 uv;
	};

	struct ColorVertex
	{
		glm::vec2 pos;
		glm::vec2 uv;
		glm::vec4 color;
	};

	void saveBatch();

	std::vector<Batch> mBatches;
	std::vector<std::uint16_t> mIndices;
	std::vector<SimpleVertex> mSimpleVertices;
	std::vector<ColorVertex> mColorVertices;
	unsigned mVertexOffset;
	unsigned mVertexCount;
	unsigned mIndexOffset;
	unsigned mIndexCount;

	Shader mTextShader;
	Shader mLevelShader;
	Shader mParticleShader;
	Shader mSpriteShader;

	GLuint mVAO;
	GLuint mVBO;
	GLuint mEBO;
};
