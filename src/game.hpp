#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "effect.hpp"
#include "eventqueue.hpp"
#include "level.hpp"
#include "resources.hpp"
#include "resourceholder.hpp"

class ParticleGen;
class Postprocess;

struct Paddle
{
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec2 vel;
	glm::vec3 color;
	bool dead;
	Texture2D texture;
};

struct Ball
{
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec2 vel;
	glm::vec3 color;
	bool stuck;
	Texture2D texture;
};

struct PowerUP
{
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec2 vel;
	glm::vec3 color;
	bool dead;
	enum Type
	{
		SPEED,
		STICKY,
		PASSTHROUGH,
		PAD_INCREASE,
		CONFUSE,
		CHAOS,
	} type;
	Texture2D texture;
};

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
	std::vector<GameLevel> mLevels;
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

	bool mKeys[1024];
};
