#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "eventqueue.hpp"
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

	std::vector<GameLevel> Levels;
	std::vector<PowerUP> PowerUPs;

	GLuint Level;
	GLuint Lives;

	Game();
	~Game();

	void run();
	void processInput();

	// void ProcessInput(GLfloat dt);
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
	float shakeTime;

private:
	void loadAssets();

	GLFWwindow *mWindow;

	EventQueue mEventQueue;
	TextureHolder mTextures;
	ShaderHolder mShaders;

	bool mKeys[1024];
};
