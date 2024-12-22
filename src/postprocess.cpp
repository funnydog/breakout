#include <iostream>

#include "glcheck.hpp"
#include "postprocess.hpp"

Postprocess::Postprocess(unsigned width, unsigned height)
	: Confuse(false)
	, Chaos(false)
	, Shake(false)
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
}

Postprocess::~Postprocess()
{
	glCheck(glDeleteRenderbuffers(1, &mRBO));
	glCheck(glDeleteFramebuffers(1, &mFBO));
	glCheck(glDeleteFramebuffers(1, &mMSFBO));
}

void
Postprocess::beginRender()
{
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO));
	glCheck(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	glCheck(glClear(GL_COLOR_BUFFER_BIT));
}

void
Postprocess::endRender()
{
	glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO));
	glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO));
	glCheck(glBlitFramebuffer(0, 0, mWidth, mHeight,
	                          0, 0, mWidth, mHeight,
	                          GL_COLOR_BUFFER_BIT, GL_NEAREST));
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void
Postprocess::bind(int textureUnit) const
{
	mTexture.bind(textureUnit);
}
