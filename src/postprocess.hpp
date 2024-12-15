#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "shader.hpp"
#include "sprite.hpp"

class Postprocess
{
public:
	Shader const& PostProcessingShader;
	Texture2D Texture;

	unsigned Width;
	unsigned Height;

	bool Confuse;
	bool Chaos;
	bool Shake;

	Postprocess(Shader const& shader, unsigned width, unsigned height);

	void BeginRender();
	void EndRender();
	void Render(float time);

private:
	GLuint MSFBO;		// multisampled FBO
	GLuint FBO;		// regular FBO
	GLuint RBO;
	GLuint VAO;
};

#endif
