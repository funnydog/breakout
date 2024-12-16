#include "glcheck.hpp"
#include "particle.hpp"

ParticleGen::ParticleGen(const Shader &shader, Texture2D texture, unsigned amount)
	: mParticles()
	, mShader(shader)
	, mTexture(texture)
	, mAmount(amount)
	, mLastUsedParticle(0)
{
	static const float quad[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
	};

	// create the VBO
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW));

	// create the VAO
	glCheck(glGenVertexArrays(1, &mVAO));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));

	// unbind the VAO and the VBO
	glCheck(glBindVertexArray(0));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

	mParticles.resize(mAmount, Particle());
}

ParticleGen::~ParticleGen()
{
	glCheck(glDeleteBuffers(1, &mVBO));
	glCheck(glDeleteVertexArrays(1, &mVAO));
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
		p.Life -= dt;
		if (p.Life > 0.0f)
		{
			p.Position -= p.Velocity * dt;
			p.Color.a -= dt * 2.5f;
		}
	}
}

void
ParticleGen::draw()
{
	mShader.use();

	// additive blending for glow effect
	glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE));

        // draw the geometry
	glCheck(glBindVertexArray(mVAO));
	for (const Particle &p : mParticles)
	{
		if (p.Life > 0.0f)
		{
			mShader.getUniform("offset").setVector2f(p.Position);
			mShader.getUniform("color").setVector4f(p.Color);
			mTexture.bind(0);
			glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));
		}
	}
	glCheck(glBindVertexArray(0));

	// restore the old blend function
	glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

GLuint
ParticleGen::firstUnusedParticle()
{
	for (unsigned i = mLastUsedParticle; i < mAmount; ++i)
	{
		if (mParticles[i].Life <= 0.0f)
		{
			mLastUsedParticle = i;
			return i;
		}
	}

	for (unsigned i = 0; i < mLastUsedParticle; ++i)
	{
		if (mParticles[i].Life <= 0.0f)
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

	p.Position = pos + random;
	p.Color = glm::vec4(rcolor, rcolor, rcolor, 1.0f);
	p.Life = 1.0f;
	p.Velocity = vel * 0.1f;
}
