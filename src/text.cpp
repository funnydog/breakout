#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "glcheck.hpp"
#include "text.hpp"

TextRenderer::TextRenderer(Shader const& s)
	: TextShader(s)
{
	this->TextShader.use();
	this->TextShader.getUniform("text").setInteger(0);

	glCheck(glGenVertexArrays(1, &this->VAO));
	glCheck(glBindVertexArray(this->VAO));

	glCheck(glGenBuffers(1, &this->VBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, this->VBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW));

	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
	glCheck(glBindVertexArray(0));
}

void
TextRenderer::Load(std::string font, GLuint fontSize)
{
	this->Characters.clear();

	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cerr << "ERROR::FREETYPE: Couldn't init FreeType Library\n";
		return;
	}

	FT_Face face;
	if (FT_New_Face(ft, font.c_str(), 0, &face)) {
		std::cerr << "ERROR::FREETYPE: Failed to load font: " << font << "\n";
		return;
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
		this->Characters.insert(std::pair<GLchar, Character>(c, character));
	}
	glCheck(glBindTexture(GL_TEXTURE_2D, 0));
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void
TextRenderer::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	this->TextShader.use();
	this->TextShader.getUniform("textColor").setVector3f(color);
	glCheck(glActiveTexture(GL_TEXTURE0));
	glBindVertexArray(this->VAO);

	for (auto c : text)
	{
		Character &ch = Characters[c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		GLfloat vertices[6][4] = {
			{ xpos, ypos + h, 0.0f, 1.0f },
			{ xpos + w, ypos, 1.0f, 0.0f },
			{ xpos, ypos, 0.0f, 0.0f },

			{ xpos, ypos + h, 0.0f, 1.0f },
			{ xpos + w, ypos + h, 1.0f, 1.0f },
			{ xpos + w, ypos, 1.0f, 0.0f },
		};

		glCheck(glBindTexture(GL_TEXTURE_2D, ch.TextureID));
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, this->VBO));
		glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));

		glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

		glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));

		x += (ch.Advance >> 6) * scale;
	}
	glCheck(glBindVertexArray(0));
	glCheck(glBindTexture(GL_TEXTURE_2D, 0));
}
