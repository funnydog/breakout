#include <iostream>

#include "glcheck.hpp"
#include "postprocess.hpp"

Postprocess::Postprocess(const Shader &shader, unsigned width, unsigned height)
	: Confuse(false)
	, Chaos(false)
	, Shake(false)
	, mShader(shader)
	, mTexture()
	, mWidth(width)
	, mHeight(height)
{
	glCheck(glGenFramebuffers(1, &mMSFBO));
	glCheck(glGenFramebuffers(1, &mFBO));
	glCheck(glGenRenderbuffers(1, &mRBO));

	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO));
	glCheck(glBindRenderbuffer(GL_RENDERBUFFER, mRBO));
	glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGB, width, height));
	glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRBO));
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Postprocess(): Failed to initialize MSFBO");
	}

	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, mFBO));
	mTexture.create(width, height, nullptr, true, true);
	glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
	                               mTexture.getHandle(), 0));
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Postprocess: Failed to initialize FBO");
	}

	// generate a VBO and upload the vertices
	static const float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,

		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
	};
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

	// generate a VAO and set the parameters
	glCheck(glGenVertexArrays(1, &mVAO));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));

        // unbind the VBO and the VAO
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
	glCheck(glBindVertexArray(0));

	// set the shader uniforms
	mShader.use();
	mShader.getUniform("scene").setInteger(0);

	GLfloat offset = 1.0f / 300.0f;
	const GLfloat offsets[][2] = {
		{ -offset,  offset },
		{    0.0f,  offset },
		{  offset,  offset },
		{ -offset,  0.0f   },
		{    0.0f,  0.0f   },
		{  offset,  0.0f   },
		{ -offset, -offset },
		{    0.0f, -offset },
		{  offset, -offset },
	};
	mShader.getUniform("offsets").setVector2fv(offsets, 9);

	GLint edge_kernel[9] = {
		-1, -1, -1,
		-1,  8, -1,
		-1, -1, -1,
	};
	mShader.getUniform("edge_kernel").setInteger1iv(edge_kernel, 9);

	GLfloat blur_kernel[9] = {
		1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
		2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
		1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
	};
	mShader.getUniform("blur_kernel").setFloat1fv(blur_kernel, 9);
}

Postprocess::~Postprocess()
{
	glCheck(glDeleteVertexArrays(1, &mVAO));
	glCheck(glDeleteBuffers(1, &mVBO));
	glCheck(glDeleteRenderbuffers(1, &mRBO));
	glCheck(glDeleteFramebuffers(1, &mFBO));
	glCheck(glDeleteFramebuffers(1, &mMSFBO));
}

void
Postprocess::BeginRender()
{
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO));
	glCheck(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	glCheck(glClear(GL_COLOR_BUFFER_BIT));
}

void
Postprocess::EndRender()
{
	glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO));
	glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO));
	glCheck(glBlitFramebuffer(0, 0, mWidth, mHeight,
	                          0, 0, mWidth, mHeight,
	                          GL_COLOR_BUFFER_BIT, GL_NEAREST));
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void
Postprocess::Render(float time)
{
	mShader.use();
	mShader.getUniform("time").setFloat(time);
	mShader.getUniform("confuse").setInteger(Confuse);
	mShader.getUniform("chaos").setInteger(Chaos);
	mShader.getUniform("shake").setInteger(Shake);

	mTexture.bind(0);
	glCheck(glBindVertexArray(mVAO));
	glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));
	glCheck(glBindVertexArray(0));
}
