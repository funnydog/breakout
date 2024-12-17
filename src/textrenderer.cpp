#include <iostream>

#include "font.hpp"
#include "glcheck.hpp"
#include "textrenderer.hpp"

TextRenderer::TextRenderer(const Shader &s)
	: mRenderer()
	, mShader(s)
{
	// set the sampler2D to texture unit 0
	mShader.use();
	mShader.getUniform("text").setInteger(0);
}

void
TextRenderer::draw(const std::string &text, glm::vec2 pos, Font &font, glm::vec3 color)
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

	mShader.use();
	mShader.getUniform("textColor").setVector3f(color);
	font.getTexture().bind(0);

	mVertices.clear();
	mRenderer.beginBatch();

	static const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	static const glm::vec2 units[] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};

	pos.y += font.getLineHeight();
	for (auto codepoint : cv.from_bytes(text))
	{
		const auto &g = font.getGlyph(codepoint);
		pos.x += g.bearing.x;
		pos.y -= g.bearing.y;
		mRenderer.reserve(4, indices);
		for (auto unit : units)
		{
			mVertices.push_back(glm::vec4(g.size*unit+pos,
			                              g.uvSize*unit+g.uvPos));
		}
		pos.x += g.advance - g.bearing.x;
		pos.y += g.bearing.y;
	}
	mRenderer.endBatch();

	mRenderer.bindBuffers();
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mVertices.size() * sizeof(mVertices[0]),
	                     mVertices.data(),
	                     GL_STREAM_DRAW));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	mRenderer.drawBuffers();
}
