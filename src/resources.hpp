#pragma once

enum class FontID
{
	Title,
	Subtitle,
};

enum class TextureID
{
	Face,
	Background,
	Blocks,
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
	Postprocess,
	Texture,
	UniformColor,
	VertexColor,
};

template <typename Resource, typename Identifier>
class ResourceHolder;

class Font;
typedef ResourceHolder<Font, FontID> FontHolder;

class Texture2D;
typedef ResourceHolder<Texture2D, TextureID> TextureHolder;

class Shader;
typedef ResourceHolder<Shader, ShaderID> ShaderHolder;
