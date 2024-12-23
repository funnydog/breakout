#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "effect.hpp"
#include "entities.hpp"
#include "eventqueue.hpp"
#include "resources.hpp"
#include "resourceholder.hpp"

class ParticleGen;
class Postprocess;
class Renderer;

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
	bool loadLevel(const std::filesystem::path &path, Level &level);
	void resetLevel();
	void resetPlayer();

	void doCollisions();

	void activatePowerUP(enum PowerUP::Type type);
	void spawnPowerUPs(glm::vec2 pos);
	void updatePowerUPs(float dt);

private:
	enum class State
	{
		Active,
		Menu,
		Win,
	} mState;

	// world data
	std::vector<Level> mLevels;
	std::vector<PowerUP> mPowerUPs;
	Paddle mPlayer;
	Ball mBall;
	// GameObject mPlayer;
	// BallObject mBall;
	unsigned mCurrentLevel;
	unsigned mLives;

	// time limited effects
	Effect mShakeEffect;
	Effect mStickyEffect;
	Effect mPassThroughEffect;
	Effect mConfuseEffect;
	Effect mChaosEffect;

	// graphics rendering data
	GLFWwindow *mWindow;
	std::unique_ptr<Renderer> mRenderer;
	std::unique_ptr<ParticleGen> mBallParticles;
	std::unique_ptr<Postprocess> mEffects;

	// support data
	EventQueue mEventQueue;
	TextureHolder mTextures;
	ShaderHolder mShaders;
	FontHolder mFonts;
};
