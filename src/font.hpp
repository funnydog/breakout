#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "texture.hpp"

struct Glyph
{
	glm::vec2 uvPos;
	glm::vec2 uvSize;
	glm::vec2 size;
	glm::vec2 bearing;
	float advance;
};

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

	glm::vec2 getSize(const std::string &text);

	const Glyph& getGlyph(char32_t codepoint);
	const Texture2D& getTexture() const;
	float getLineHeight() const;

private:
	void resizeTexture(unsigned newWidth, unsigned newHeight);

	std::unordered_map<char32_t, Glyph> mGlyphs;
	std::vector<std::uint8_t> mPixelBuffer;
	Texture2D mTexture;

	FT_Library mFT;
	FT_Face mFace;
	float mLineHeight;
	int mPositionX;
	int mPositionY;
	int mMaxHeight;
};
