#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "font.hpp"
#include "glcheck.hpp"
#include "particle.hpp"
#include "postprocess.hpp"

#include "renderer.hpp"

namespace
{
static const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
static const glm::vec2 units[] = {
	{ 0.f, 0.f },
	{ 0.f, 1.f },
	{ 1.f, 0.f },
	{ 1.f, 1.f },
};
static const float fullquad[][4] = {
	{ -1.f,  1.f, 0.f, 1.f },
	{ -1.f, -1.f, 0.f, 0.f },
	{  1.f,  1.f, 1.f, 1.f },
	{  1.f, -1.f, 1.f, 0.f },
};
static const float offset = 1.f / 256.f;
static const float offsets[][2] = {
	{ -offset,  offset },
	{    0.0f,  offset },
	{  offset,  offset },
	{ -offset,  0.0f   },
	{    0.0f,  0.0f   },
	{  offset,  0.0f   },
	{ -offset, -offset },
	{    0.0f, -offset },
	{  offset, -offset },
};
static const GLint edge_kernel[9] = {
	-1, -1, -1,
	-1,  8, -1,
	-1, -1, -1,
};
static const float blur_kernel[9] = {
	1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
	2.f / 16.f, 4.f / 16.f, 2.f / 16.f,
	1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
};
}

Renderer::Renderer(unsigned screenWidth, unsigned screenHeight, const ShaderHolder &shaders)
	: mVertexOffset(0)
	, mVertexCount(0)
	, mIndexOffset(0)
	, mIndexCount(0)
	, mPostShader(shaders.get(ShaderID::Postprocess))
	, mTextureShader(shaders.get(ShaderID::Texture))
	, mUniformColorShader(shaders.get(ShaderID::UniformColor))
	, mVertexColorShader(shaders.get(ShaderID::VertexColor))
{
	// bind a buffer to allow calling glVertexAttribPointer()
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));

	glCheck(glGenBuffers(1, &mEBO));
	glCheck(glGenVertexArrays(1, &mSimpleVAO));

	// for mSimpleVAO we have only one packed attrib pointer
	glCheck(glBindVertexArray(mSimpleVAO));
	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));

	// for mColorVAO we have two attrib pointers
	glCheck(glGenVertexArrays(1, &mColorVAO));
	glCheck(glBindVertexArray(mColorVAO));
	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
	                              sizeof(ColorVertex),
	                              reinterpret_cast<GLvoid*>(offsetof(ColorVertex, pos))));
	glCheck(glEnableVertexAttribArray(1));
	glCheck(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
	                              sizeof(ColorVertex),
	                              reinterpret_cast<GLvoid*>(offsetof(ColorVertex, color))));

	// create the orthographic projection matrix
	glm::mat4 proj = glm::ortho(
		0.0f, static_cast<GLfloat>(screenWidth),
		static_cast<GLfloat>(screenHeight), 0.0f,
		-1.0f, 1.0f);

	// configure the shaders
	mPostShader.use();
	mPostShader.getUniform("scene").setInteger(0);
	mPostShader.getUniform("offsets").setVector2fv(offsets, 9);
	mPostShader.getUniform("edge_kernel").setInteger1iv(edge_kernel, 9);
	mPostShader.getUniform("blur_kernel").setFloat1fv(blur_kernel, 9);

	mTextureShader.use();
	mTextureShader.getUniform("image").setInteger(0);
	mTextureShader.getUniform("projection").setMatrix4(proj);

	mUniformColorShader.use();
	mUniformColorShader.getUniform("image").setInteger(0);
	mUniformColorShader.getUniform("projection").setMatrix4(proj);

	mVertexColorShader.use();
	mVertexColorShader.getUniform("image").setInteger(0);
	mVertexColorShader.getUniform("projection").setMatrix4(proj);
}

Renderer::~Renderer()
{
	glCheck(glBindVertexArray(0));
	glCheck(glDeleteVertexArrays(1, &mColorVAO));
	glCheck(glDeleteVertexArrays(1, &mSimpleVAO));
	glCheck(glDeleteBuffers(1, &mEBO));
	glCheck(glDeleteBuffers(1, &mVBO));
}

void
Renderer::clear(glm::vec4 color) const
{
	glCheck(glClearColor(color.r, color.g, color.b, 1.0f));
	glCheck(glClear(GL_COLOR_BUFFER_BIT));
}

void
Renderer::draw(const std::string &text, glm::vec2 pos, Font &font, glm::vec3 color)
{
	if (text.empty())
	{
		return;
	}

	// ensure the needed glyphs are rendered
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
	for (auto codepoint: cv.from_bytes(text))
	{
		font.getGlyph(codepoint);
	}

	mUniformColorShader.use();
	mUniformColorShader.getUniform("uniformColor").setVector3f(color);
	font.getTexture().bind(0);

	mSimpleVertices.clear();
	beginBatch();
	pos.y += font.getLineHeight();
	for (auto codepoint : cv.from_bytes(text))
	{
		const auto &g = font.getGlyph(codepoint);
		pos.x += g.bearing.x;
		pos.y -= g.bearing.y;
		reserve(4, indices);
		for (auto unit : units)
		{
			SimpleVertex v;
			v.pos = g.size * unit + pos;
			v.uv = g.uvSize * unit + g.uvPos;
			mSimpleVertices.push_back(v);
		}
		pos.x += g.advance - g.bearing.x;
		pos.y += g.bearing.y;
	}
	endBatch();

	glCheck(glBindVertexArray(mSimpleVAO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mSimpleVertices.size() * sizeof(mSimpleVertices[0]),
	                     mSimpleVertices.data(),
	                     GL_STREAM_DRAW));
	drawBuffers();
}

void
Renderer::draw(const Level &level)
{
	static constexpr glm::vec2 uvSize = { 128.f/1024.f, 1.f };
	static constexpr glm::vec2 uvPos[] = {
		{0 * 128.f/1024.f, 0.f},
		{1 * 128.f/1024.f, 0.f},
		{2 * 128.f/1024.f, 0.f},
		{3 * 128.f/1024.f, 0.f},
		{4 * 128.f/1024.f, 0.f},
		{5 * 128.f/1024.f, 0.f},
	};

	mSimpleVertices.clear();
	beginBatch();
	glm::vec2 blockSize = level.blockSize;
	for (const auto &b : level.blocks)
	{
		if (b.dead)
		{
			continue;
		}
		reserve(4, indices);
		for (auto unit : units)
		{
			SimpleVertex v;
			v.pos = blockSize * unit + b.position;
			v.uv = uvSize * unit + uvPos[b.type];
			mSimpleVertices.push_back(v);
		}
	}
	endBatch();

	glCheck(glBindVertexArray(mSimpleVAO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mSimpleVertices.size() * sizeof(mSimpleVertices[0]),
	                     mSimpleVertices.data(),
	                     GL_STREAM_DRAW));

	mTextureShader.use();
	level.texture.bind(0);
	drawBuffers();
}

void
Renderer::draw(const ParticleGen &pg)
{
	mColorVertices.clear();
	auto size = pg.getParticleSize();
	beginBatch();
	for (const auto &p : pg.getParticles())
	{
		if (p.life <= 0.f)
		{
			continue;
		}
		reserve(4, indices);
		for (auto unit : units)
		{
			ColorVertex v;
			v.pos = size * unit + p.position;
			v.uv = unit;
			v.color = p.color;
			mColorVertices.push_back(v);
		}
	}
	endBatch();

	glCheck(glBindVertexArray(mColorVAO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mColorVertices.size() * sizeof(mColorVertices[0]),
	                     mColorVertices.data(),
	                     GL_STREAM_DRAW));
	mVertexColorShader.use();
	pg.getTexture().bind(0);

	// set an additive blending for the glow effect
	glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE));

	drawBuffers();

	// restore the previous blend function
	glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	glCheck(glDisableVertexAttribArray(1));
}

void
Renderer::draw(const Postprocess &pp, float time)
{
	beginBatch();
	reserve(4, indices);
	endBatch();

	glCheck(glBindVertexArray(mSimpleVAO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(fullquad), fullquad,
	                     GL_STREAM_DRAW));

	mPostShader.use();
	mPostShader.getUniform("time").setFloat(time);
	mPostShader.getUniform("confuse").setInteger(pp.Confuse);
	mPostShader.getUniform("chaos").setInteger(pp.Chaos);
	mPostShader.getUniform("shake").setInteger(pp.Shake);
	pp.bind(0);
	drawBuffers();
}

void
Renderer::draw(Texture2D texture, glm::vec2 position, glm::vec2 size, glm::vec3 color)
{
	mSimpleVertices.clear();
	beginBatch();
	reserve(4, indices);
	for (auto unit : units)
	{
		SimpleVertex v;
		v.pos = size * unit + position;
		v.uv = unit;
		mSimpleVertices.push_back(v);
	}
	endBatch();

	glCheck(glBindVertexArray(mSimpleVAO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mSimpleVertices.size()*sizeof(mSimpleVertices[0]),
	                     mSimpleVertices.data(),
	                     GL_STREAM_DRAW));
	mUniformColorShader.use();
	mUniformColorShader.getUniform("uniformColor").setVector3f(color);

	texture.bind(0);
	drawBuffers();
}

void
Renderer::draw(const Paddle &paddle)
{
	draw(paddle.texture, paddle.pos, paddle.size, paddle.color);
}
void
Renderer::draw(const Ball &ball)
{
	draw(ball.texture, ball.pos, ball.size, ball.color);
}

void
Renderer::draw(const PowerUP &pow)
{
	draw(pow.texture, pow.pos, pow.size, pow.color);
}

void
Renderer::saveBatch()
{
	mBatches.emplace_back(mVertexOffset, mIndexOffset, mIndexCount-mIndexOffset);
	mVertexOffset = mVertexCount;
	mIndexOffset = mIndexCount;
}

void
Renderer::reserve(unsigned vcount, std::span<const std::uint16_t> indices)
{
	auto base = mVertexCount - mVertexOffset;
	if (base + vcount > UINT16_MAX)
	{
		saveBatch();
		base = 0;
	}
	mVertexCount += vcount;
	for (auto i : indices)
	{
		mIndices.push_back(base + i);
	}
	mIndexCount += indices.size();
}

void
Renderer::beginBatch()
{
	mIndices.clear();
	mBatches.clear();
	mVertexOffset = mVertexCount = 0;
	mIndexOffset = mIndexCount = 0;
}

void
Renderer::endBatch()
{
	saveBatch();
}

void
Renderer::drawBuffers() const
{
	// upload the indices
	glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO));
	glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			     mIndices.size() * sizeof(mIndices[0]),
			     mIndices.data(),
			     GL_STREAM_DRAW));

	// draw all the batches
	for (const auto &batch : mBatches)
	{
		glCheck(glDrawElementsBaseVertex(
			        GL_TRIANGLES,
			        batch.indexCount,
			        GL_UNSIGNED_SHORT,
			        reinterpret_cast<GLvoid*>(batch.indexOffset),
			        batch.vertexOffset));
	}
}
