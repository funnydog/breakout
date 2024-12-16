#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "textrenderer.hpp"
#include "texture.hpp"

class Font
{
public:
	Font();
	~Font();

	Font(const Font&) = delete;
	Font& operator=(const Font&) = delete;
	Font(Font&&) noexcept = delete;
	Font& operator=(Font&&) noexcept = delete;

	bool loadFromFile(const std::filesystem::path &path, unsigned size);

	void draw(TextRenderer &renderer, const std::string &text, glm::vec2 pos);

	glm::vec2 getSize(const std::string &text);

private:
	struct Glyph
	{
		glm::vec2 uvPos;
		glm::vec2 uvSize;
		glm::vec2 size;
		glm::vec2 bearing;
		float advance;
	};

	void resizeTexture(unsigned newWidth, unsigned newHeight);
	const Glyph& getGlyph(char32_t codepoint);

	std::unordered_map<char32_t, Glyph> mGlyphs;
	std::vector<std::uint8_t> mPixelBuffer;
	Texture2D mTexture;

	FT_Library mFT;
	FT_Face mFace;
	int mLineHeight;
	int mPositionX;
	int mPositionY;
	int mMaxHeight;
};
