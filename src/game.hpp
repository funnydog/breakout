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

class Game
{
public:
	Game();
	~Game();

	void run();
	void processInput();

	void handleEvent(const Event &event);
	void update(float dt);
	void render();

private:
	void loadAssets();
	void resetLevel();
	void resetPlayer();

	void doCollisions();

	void activatePowerUP(PowerUP &p);
	void spawnPowerUPs(glm::vec2 pos);
	void updatePowerUPs(float dt);

private:
	enum class State
	{
		Active,
		Menu,
		Win,
	} mState;

	std::vector<GameLevel> mLevels;
	std::vector<PowerUP> mPowerUPs;
	std::unique_ptr<Renderer> mRenderer;
	std::unique_ptr<GameObject> mPlayer;
	std::unique_ptr<BallObject> mBall;
	std::unique_ptr<ParticleGen> mBallParticles;
	std::unique_ptr<Postprocess> mEffects;

	float mShakeTime;
	unsigned mCurrentLevel;
	unsigned mLives;

	GLFWwindow *mWindow;

	EventQueue mEventQueue;
	TextureHolder mTextures;
	ShaderHolder mShaders;
	FontHolder mFonts;

	bool mKeys[1024];
};
