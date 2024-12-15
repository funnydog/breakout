#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.hpp"
#include "texture.hpp"
#include "gameobject.hpp"

struct Particle
{
	glm::vec2 Position;
	glm::vec2 Velocity;
	glm::vec4 Color;
	float Life;

	Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};

class ParticleGen
{
public:
	ParticleGen(Shader const& shader, Texture2D texture, GLuint amount);

	void Update(GLfloat dt, GameObject &object, GLuint newParticles, glm::vec2 offset = glm::vec2(0.0f));
	void Draw();
private:
	std::vector<Particle> particles;

	Shader shader;
	Texture2D texture;
	GLuint amount;
	GLuint VAO;
	GLuint lastUsedParticle;

	GLuint firstUnusedParticle();
	void respawnParticle(Particle &particle, GameObject &obj, glm::vec2 offset = glm::vec2(0.0f));
};
