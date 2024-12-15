#pragma once

enum class LevelID
{
	One,
	Two,
	Three,
	Four,
};

enum class FontID
{
	OcraEXT,
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

class Font;
typedef ResourceHolder<Font, FontID> FontHolder;

class Texture2D;
typedef ResourceHolder<Texture2D, TextureID> TextureHolder;

class Shader;
typedef ResourceHolder<Shader, ShaderID> ShaderHolder;
