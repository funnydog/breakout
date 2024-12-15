#include <iostream>

#include "glcheck.hpp"
#include "postprocess.hpp"

Postprocess::Postprocess(Shader const& shader, unsigned width, unsigned height) :
	PostProcessingShader(shader), Texture(), Width(width), Height(height),
	Confuse(false), Chaos(false), Shake(false)
{
	glCheck(glGenFramebuffers(1, &this->MSFBO));
	glCheck(glGenFramebuffers(1, &this->FBO));
	glCheck(glGenRenderbuffers(1, &this->RBO));

	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, this->MSFBO));
	glCheck(glBindRenderbuffer(GL_RENDERBUFFER, this->RBO));
	glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGB, width, height));
	glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, this->RBO));
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Postprocess(): Failed to initialize MSFBO");
	}

	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, this->FBO));
	this->Texture.create(width, height);
	glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->Texture.getHandle(), 0));
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Postprocess: Failed to initialize FBO");
	}

	GLuint VBO;
	static const float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,

		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
	};
	glCheck(glGenVertexArrays(1, &this->VAO));
	glCheck(glBindVertexArray(this->VAO));

	glCheck(glGenBuffers(1, &VBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, VBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
	glCheck(glBindVertexArray(0));

	this->PostProcessingShader.use();
	this->PostProcessingShader.getUniform("scene").setInteger(0);
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
	this->PostProcessingShader.getUniform("offsets").setVector2fv(offsets, 9);

	GLint edge_kernel[9] = {
		-1, -1, -1,
		-1,  8, -1,
		-1, -1, -1,
	};
	this->PostProcessingShader.getUniform("edge_kernel").setInteger1iv(edge_kernel, 9);

	GLfloat blur_kernel[9] = {
		1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
		2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
		1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
	};
	this->PostProcessingShader.getUniform("blur_kernel").setFloat1fv(blur_kernel, 9);
}

void
Postprocess::BeginRender()
{
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, this->MSFBO));
	glCheck(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	glCheck(glClear(GL_COLOR_BUFFER_BIT));
}

void
Postprocess::EndRender()
{
	glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, this->MSFBO));
	glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->FBO));
	glCheck(glBlitFramebuffer(0, 0, this->Width, this->Height,
	                          0, 0, this->Width, this->Height,
	                          GL_COLOR_BUFFER_BIT, GL_NEAREST));
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void
Postprocess::Render(float time)
{
	this->PostProcessingShader.use();
	this->PostProcessingShader.getUniform("time").setFloat(time);
	this->PostProcessingShader.getUniform("confuse").setInteger(this->Confuse);
	this->PostProcessingShader.getUniform("chaos").setInteger(this->Chaos);
	this->PostProcessingShader.getUniform("shake").setInteger(this->Shake);

	Texture.bind(0);
	glCheck(glBindVertexArray(this->VAO));
	glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));
	glCheck(glBindVertexArray(0));
}
