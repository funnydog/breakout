#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.hpp"
#include "texture.hpp"

class ParticleGen
{
public:
	ParticleGen(const Shader &shader, Texture2D texture, unsigned amount);
	~ParticleGen();

	void update(GLfloat dt, unsigned newParticles, glm::vec2 pos, glm::vec2 vel);
	void draw();
private:
	struct Particle
	{
		glm::vec2 Position;
		glm::vec2 Velocity;
		glm::vec4 Color;
		float Life;
		Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
	};

	unsigned firstUnusedParticle();
	void respawnParticle(Particle &particle, glm::vec2 pos, glm::vec2 vel);

	std::vector<Particle> mParticles;
	Shader mShader;
	Texture2D mTexture;
	GLuint mAmount;
	GLuint mVAO;
	GLuint mVBO;
	GLuint mLastUsedParticle;
};
