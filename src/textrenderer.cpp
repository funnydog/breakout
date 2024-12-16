#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "glcheck.hpp"
#include "textrenderer.hpp"

TextRenderer::TextRenderer(const Shader &s)
	: mShader(s)
{
	// set the sampler2D to texture unit 0
	mShader.use();
	mShader.getUniform("text").setInteger(0);

	// reserve a VBO for the vertices
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW));

	// save the state to a VAO
	glCheck(glGenVertexArrays(1, &mVAO));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));

        // unbind the VAO and the VBO
	glCheck(glBindVertexArray(0));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

TextRenderer::~TextRenderer()
{
	for (auto&[_, ch] : mCharacters)
	{
		glCheck(glDeleteTextures(1, &ch.TextureID));
	}
	glCheck(glDeleteVertexArrays(1, &mVAO));
	glCheck(glDeleteBuffers(1, &mVBO));
}

bool
TextRenderer::load(const std::string &font, GLuint fontSize) noexcept
{
	for (auto&[_, ch] : mCharacters)
	{
		glCheck(glDeleteTextures(1, &ch.TextureID));
	}
	mCharacters.clear();

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cerr << "ERROR::FREETYPE: Couldn't init FreeType Library\n";
		return false;
	}

	FT_Face face;
	if (FT_New_Face(ft, font.c_str(), 0, &face))
	{
		std::cerr << "ERROR::FREETYPE: Failed to load font: " << font << "\n";
		return false;
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);
	glCheck(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	for (GLubyte c = 0; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYPE: Failed to load Glyph\n";
			continue;
		}

		GLuint texture;
		glCheck(glGenTextures(1, &texture));
		glCheck(glBindTexture(GL_TEXTURE_2D, texture));
		glCheck(glTexImage2D(
			        GL_TEXTURE_2D,
			        0,
			        GL_RED,
			        face->glyph->bitmap.width,
			        face->glyph->bitmap.rows,
			        0,
			        GL_RED,
			        GL_UNSIGNED_BYTE,
			        face->glyph->bitmap.buffer
			        ));

		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			(GLuint)face->glyph->advance.x
		};
		mCharacters.insert(std::make_pair(c, character));
	}
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	return true;
}

void
TextRenderer::draw(const std::string &text, glm::vec2 pos, float scale, glm::vec3 color)
{
	mShader.use();
	mShader.getUniform("textColor").setVector3f(color);
	glCheck(glActiveTexture(GL_TEXTURE0));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));

	int hBearing = mCharacters['H'].Bearing.y;
	for (auto c : text)
	{
		Character &ch = mCharacters[c];

		glm::vec2 p;
		p.x = pos.x + ch.Bearing.x * scale;
		p.y = pos.y + (hBearing - ch.Bearing.y) * scale;

		glm::vec2 s(ch.Size.x * scale, ch.Size.y * scale);

		GLfloat vertices[6][4] = {
			{ p.x, p.y + s.y, 0.0f, 1.0f },
			{ p.x + s.x, p.y, 1.0f, 0.0f },
			{ p.x, p.y, 0.0f, 0.0f },

			{ p.x, p.y + s.y, 0.0f, 1.0f },
			{ p.x + s.x, p.y + s.y, 1.0f, 1.0f },
			{ p.x + s.x, p.y, 1.0f, 0.0f },
		};

		glCheck(glBindTexture(GL_TEXTURE_2D, ch.TextureID));
		glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
		glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));

		pos.x += (ch.Advance >> 6) * scale;
	}
	glCheck(glBindVertexArray(0));
}
