#pragma once

#include <span>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.hpp"
#include "entities.hpp"
#include "resources.hpp"
#include "resourceholder.hpp"

class Font;
class ParticleGen;
class Postprocess;
class Texture2D;

class Renderer
{
public:
	Renderer(unsigned screenWidth, unsigned screenHeight, const ShaderHolder &shaders);
	~Renderer();

	void clear(glm::vec4 color) const;
	void draw(const std::string &text, glm::vec2 pos,
	          Font &font, glm::vec3 color = glm::vec3(1.0f));

	void draw(const Paddle &player);
	void draw(const Ball &ball);
	void draw(const PowerUP &pow);
	void draw(const Level &level);
	void draw(const ParticleGen &pg);
	void draw(const Postprocess &pp, float time);

	void draw(Texture2D texture, glm::vec2 pos, glm::vec2 size,
	          glm::vec3 color = glm::vec3(1.0f));
private:

	void reserve(unsigned vcount, std::span<const std::uint16_t> indices);
	void beginBatch();
	void endBatch();
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

	Shader mPostShader;
	Shader mTextureShader;
	Shader mUniformColorShader;
	Shader mVertexColorShader;

	GLuint mSimpleVAO;
	GLuint mColorVAO;
	GLuint mVBO;
	GLuint mEBO;
};
