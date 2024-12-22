#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "shader.hpp"

class Postprocess
{
public:
	Postprocess(unsigned width, unsigned height);
	~Postprocess();

	void beginRender();
	void endRender();
	void bind(int textureUnit) const;

	bool Confuse;
	bool Chaos;
	bool Shake;

private:
	Texture2D mTexture;

	unsigned mWidth;
	unsigned mHeight;

	GLuint mMSFBO;          // multisampled FBO
	GLuint mFBO;            // regular FBO
	GLuint mRBO;            // render buffer
};
