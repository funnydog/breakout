#ifndef GAME_H
#define GAME_H

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "level.hpp"
#include "powerup.hpp"
#include "resources.hpp"
#include "resourceholder.hpp"

class BallObject;
class ParticleGen;
class Postprocess;
class TextRenderer;

class Game
{
public:
	enum class State
	{
		GAME_ACTIVE,
		GAME_MENU,
		GAME_WIN,
	} State;

	GLboolean Keys[1024];
	GLboolean KeysProcessed[1024];
	GLuint Width, Height;

	std::vector<GameLevel> Levels;
	std::vector<PowerUP> PowerUPs;

	GLuint Level;
	GLuint Lives;

	Game(GLuint width, GLuint height);
	~Game();

	void Init();
	void ProcessInput(GLfloat dt);
	void Update(GLfloat dt);
	void Render();

	void ResetLevel();
	void ResetPlayer();

	void DoCollisions();

	void ActivatePowerUP(PowerUP &p);
	void SpawnPowerUPs(GameObject &block);
	void UpdatePowerUPs(float dt);

private:
	std::unique_ptr<SpriteRenderer> renderer;
	std::unique_ptr<GameObject> player;
	std::unique_ptr<BallObject> ball;
	std::unique_ptr<ParticleGen> particles;
	std::unique_ptr<Postprocess> effects;
	std::unique_ptr<TextRenderer> text;

private:
	void loadAssets();

	TextureHolder mTextures;
	ShaderHolder mShaders;

	float shakeTime;
};

#endif
