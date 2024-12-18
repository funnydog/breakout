#include "glcheck.hpp"
#include "particle.hpp"

void
ParticleGen::set(Texture2D tex, unsigned amount)
{
	mTexture = tex;
	mAmount = amount;
	mParticles.resize(mAmount);
}

void
ParticleGen::update(GLfloat dt, unsigned newParticles, glm::vec2 pos, glm::vec2 vel)
{
	// new particles
	while (newParticles-->0)
	{
		int unusedParticle = firstUnusedParticle();
		respawnParticle(mParticles[unusedParticle], pos, vel);
	}

	// update particles
	for (Particle &p : mParticles)
	{
		p.life -= dt;
		if (p.life > 0.0f)
		{
			p.position -= p.velocity * dt;
			p.color.a -= dt * 2.5f;
		}
	}
}

const std::vector<Particle> &
ParticleGen::getParticles() const
{
	return mParticles;
}

const Texture2D &
ParticleGen::getTexture() const
{
	return mTexture;
}

glm::vec2
ParticleGen::getParticleSize() const
{
	return glm::vec2(10.f);
}

GLuint
ParticleGen::firstUnusedParticle()
{
	for (unsigned i = mLastUsedParticle; i < mAmount; ++i)
	{
		if (mParticles[i].life <= 0.0f)
		{
			mLastUsedParticle = i;
			return i;
		}
	}

	for (unsigned i = 0; i < mLastUsedParticle; ++i)
	{
		if (mParticles[i].life <= 0.0f)
		{
			mLastUsedParticle = i;
			return i;
		}
	}

	mLastUsedParticle = 0;
	return 0;
}

void
ParticleGen::respawnParticle(Particle &p, glm::vec2 pos, glm::vec2 vel)
{
	float random = ((rand() % 100) - 50) / 10.0f;
	float rcolor = 0.5 + ((rand() % 100) / 100.0f);

	p.position = pos + random;
	p.color = glm::vec4(rcolor, rcolor, rcolor, 1.0f);
	p.life = 1.0f;
	p.velocity = vel * 0.1f;
}
