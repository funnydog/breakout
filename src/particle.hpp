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
	ParticleGen(Texture2D texture, unsigned amount);

	void update(float dt, unsigned newParticles, glm::vec2 pos, glm::vec2 vel);

	const std::vector<Particle> &getParticles() const;
	const Texture2D &getTexture() const;
	glm::vec2 getParticleSize() const;

private:

	unsigned firstUnusedParticle();
	void respawnParticle(Particle &particle, glm::vec2 pos, glm::vec2 vel);

	std::vector<Particle> mParticles;
	Texture2D mTexture;
	unsigned mAmount;
	unsigned mLastUsedParticle;
};
