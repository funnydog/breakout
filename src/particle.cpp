#include "particle.hpp"

ParticleGen::ParticleGen(Shader const& shader, Texture2D texture, GLuint amount) :
	shader(shader), texture(texture), amount(amount), lastUsedParticle(0)
{
	GLuint VBO;
	static const float quad[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &this->VAO);
	glBindVertexArray(this->VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);

	this->particles.resize(this->amount, Particle());
}

void
ParticleGen::Update(GLfloat dt, GameObject &obj, GLuint newParticles, glm::vec2 offset)
{
	// new particles
	for (unsigned i = 0; i < newParticles; i++) {
		int unusedParticle = this->firstUnusedParticle();
		respawnParticle(this->particles[unusedParticle], obj, offset);
	}

	// update particles
	for (Particle &p : this->particles) {
		p.Life -= dt;
		if (p.Life > 0.0f) {
			p.Position -= p.Velocity * dt;
			p.Color.a -= dt * 2.5f;
		}
	}
}

void
ParticleGen::Draw()
{
	// additive blending for glow effect
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	shader.use();
	glBindVertexArray(this->VAO);
	for (Particle &p : this->particles)
	{
		if (p.Life <= 0.0f)
			continue;

		this->shader.getUniform("offset").setVector2f(p.Position);
		this->shader.getUniform("color").setVector4f(p.Color);
		Texture2D::bind(&texture, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glBindVertexArray(0);

	// restore the old blend function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

GLuint
ParticleGen::firstUnusedParticle()
{
	for (unsigned i = this->lastUsedParticle; i < this->amount; ++i) {
		if (this->particles[i].Life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}

	for (unsigned i = 0; i < lastUsedParticle; ++i) {
		if (this->particles[i].Life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}

	lastUsedParticle = 0;
	return 0;
}

void
ParticleGen::respawnParticle(Particle &p, GameObject &obj, glm::vec2 offset)
{
	float random = ((rand() % 100) - 50) / 10.0f;
	float rcolor = 0.5 + ((rand() % 100) / 100.0f);

	p.Position = obj.Position + random + offset;
	p.Color = glm::vec4(rcolor, rcolor, rcolor, 1.0f);
	p.Life = 1.0f;
	p.Velocity = obj.Velocity * 0.1f;
}
