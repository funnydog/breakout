#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"

struct Particle
{
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec4 color;
	float life = 0.f;
};

class ParticleGen
{
public:
	void set(Texture2D tex, unsigned amount);
	void update(GLfloat dt, unsigned newParticles, glm::vec2 pos, glm::vec2 vel);

	const std::vector<Particle> &getParticles() const;
	const Texture2D &getTexture() const;
	glm::vec2 getParticleSize() const;

private:
	unsigned firstUnusedParticle();
	void respawnParticle(Particle &particle, glm::vec2 pos, glm::vec2 vel);

private:
	std::vector<Particle> mParticles;
	Texture2D mTexture;
	unsigned mAmount = 0;
	unsigned mLastUsedParticle = 0;
};
