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
class ParticleRenderer;
class LevelRenderer;

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
	void update(GLfloat dt);
	void render();

	void ResetLevel();
	void ResetPlayer();

	void DoCollisions();

	void ActivatePowerUP(PowerUP &p);
	void SpawnPowerUPs(glm::vec2 pos);
	void UpdatePowerUPs(float dt);

private:
	std::unique_ptr<SpriteRenderer> renderer;
	std::unique_ptr<BatchRenderer> mBatchRenderer;
	std::unique_ptr<LevelRenderer> mLevelRenderer;
	std::unique_ptr<ParticleRenderer> mParticleRenderer;
	std::unique_ptr<GameObject> player;
	std::unique_ptr<BallObject> ball;
	std::unique_ptr<ParticleGen> particles;
	std::unique_ptr<Postprocess> effects;
	float shakeTime;

private:
	void loadAssets();

	GLFWwindow *mWindow;

	EventQueue mEventQueue;
	TextureHolder mTextures;
	ShaderHolder mShaders;
	FontHolder mFonts;

	bool mKeys[1024];
};
