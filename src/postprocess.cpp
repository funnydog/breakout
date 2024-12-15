#include <iostream>

#include "postprocess.h"

Postprocess::Postprocess(Shader const& shader, unsigned width, unsigned height) :
	PostProcessingShader(shader), Texture(), Width(width), Height(height),
	Confuse(false), Chaos(false), Shake(false)
{
	glGenFramebuffers(1, &this->MSFBO);
	glGenFramebuffers(1, &this->FBO);
	glGenRenderbuffers(1, &this->RBO);

	glBindFramebuffer(GL_FRAMEBUFFER, this->MSFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGB, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, this->RBO);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR::Postprocess: Failed to initialize MSFBO\n";

	glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
        // TODO: check the return value
	this->Texture.create(width, height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->Texture.getHandle(), 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR::Postprocess: Failed to initialize FBO\n";

	GLuint VBO;
	static const float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,

		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
	};
	glGenVertexArrays(1, &this->VAO);
	glBindVertexArray(this->VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	this->PostProcessingShader.Use();
	this->PostProcessingShader.GetUniform("scene").SetInteger(0);
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
	this->PostProcessingShader.GetUniform("offsets").SetVector2fv(offsets, 9);

	GLint edge_kernel[9] = {
		-1, -1, -1,
		-1,  8, -1,
		-1, -1, -1,
	};
	this->PostProcessingShader.GetUniform("edge_kernel").SetInteger1iv(edge_kernel, 9);

	GLfloat blur_kernel[9] = {
		1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
		2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
		1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
	};
	this->PostProcessingShader.GetUniform("blur_kernel").SetFloat1fv(blur_kernel, 9);
}

void
Postprocess::BeginRender()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->MSFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void
Postprocess::EndRender()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, this->MSFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->FBO);
	glBlitFramebuffer(0, 0, this->Width, this->Height,
			  0, 0, this->Width, this->Height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
Postprocess::Render(float time)
{
	this->PostProcessingShader.Use();
	this->PostProcessingShader.GetUniform("time").SetFloat(time);
	this->PostProcessingShader.GetUniform("confuse").SetInteger(this->Confuse);
	this->PostProcessingShader.GetUniform("chaos").SetInteger(this->Chaos);
	this->PostProcessingShader.GetUniform("shake").SetInteger(this->Shake);

	glActiveTexture(GL_TEXTURE0);
	this->Texture.bind();
	glBindVertexArray(this->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
