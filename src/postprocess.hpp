#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "shader.hpp"
#include "spriterenderer.hpp"

class Postprocess
{
public:
	Postprocess(const Shader &shader, unsigned width, unsigned height);
	~Postprocess();

	void BeginRender();
	void EndRender();
	void Render(float time);

	bool Confuse;
	bool Chaos;
	bool Shake;

private:
	Shader mShader;
	Texture2D mTexture;

	unsigned mWidth;
	unsigned mHeight;

	GLuint mMSFBO;          // multisampled FBO
	GLuint mFBO;            // regular FBO
	GLuint mRBO;
	GLuint mVAO;
	GLuint mVBO;
};
