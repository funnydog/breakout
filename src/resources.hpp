#pragma once

#include "font.hpp"
#include "shader.hpp"
#include "texture.hpp"

enum class LevelID
{
	One,
	Two,
	Three,
	Four,
};

enum class FontID
{
	Title,
	Subtitle,
};

enum class TextureID
{
	Face,
	Background,
	Block,
	BlockSolid,
	Paddle,
	Particle,
	PowerupSpeed,
	PowerupSticky,
	PowerupIncrease,
	PowerupConfuse,
	PowerupChaos,
	PowerupPassthrough,
};

enum class ShaderID
{
	Particle,
	Postprocess,
	Sprite,
	Text,
};

template <typename Resource, typename Identifier>
class ResourceHolder;

typedef ResourceHolder<Font, FontID> FontHolder;

typedef ResourceHolder<Texture2D, TextureID> TextureHolder;

typedef ResourceHolder<Shader, ShaderID> ShaderHolder;
