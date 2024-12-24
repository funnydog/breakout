#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "font.hpp"
#include "game.hpp"
#include "glcheck.hpp"
#include "particle.hpp"
#include "postprocess.hpp"
#include "renderer.hpp"

namespace
{
static constexpr glm::vec2 PlayerSize(100, 20);
static constexpr glm::vec2 PlayerVelocity(500.f, 0.f);
static constexpr glm::vec2 InitialBallVelocity(100.0f, -350.0f);
static constexpr float BallRadius = 12.5f;
static constexpr glm::vec2 PowerUPSize(60, 20);
static constexpr glm::vec2 PowerUPVelocity(0.0f, 150.0f);
static constexpr unsigned InitialLives = 3;

static constexpr std::string_view levels[] = {
	"assets/levels/one.txt",
	"assets/levels/two.txt",
	"assets/levels/three.txt",
	"assets/levels/four.txt",
};

const unsigned ScreenWidth = 800;
const unsigned ScreenHeight = 600;
}

Game::Game()
	: mState(State::Menu)
	, mCurrentLevel(0)
	, mLives(InitialLives)
	, mWindow(nullptr)
{
	const char *error;
	if (!glfwInit())
	{
		glfwGetError(&error);
		throw std::runtime_error(error);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	mWindow = glfwCreateWindow(
		ScreenWidth, ScreenHeight, "Breakout",
		nullptr, nullptr);
	if (!mWindow)
	{
		glfwGetError(&error);
		throw std::runtime_error(error);
	}
	glfwMakeContextCurrent(mWindow);
	glewExperimental = GL_TRUE;

	glCheck(glewInit());
	glCheck(glViewport(0, 0, ScreenWidth, ScreenHeight));
	glCheck(glEnable(GL_CULL_FACE));
	glCheck(glEnable(GL_BLEND));
	glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	// audio device
	if (!mAudioDevice.open(""))
	{
		throw std::runtime_error("Cannot open the audio device");
	}
	mAudioDevice.setMasterVolume(50.f);

	mEventQueue.track(mWindow);

	// load the assets
	loadAssets();

	// make the batch renderer
	mRenderer = std::make_unique<Renderer>(ScreenWidth, ScreenHeight, mShaders);

	// set-up the effects
	mEffects = std::make_unique<Postprocess>(
		ScreenWidth,
		ScreenHeight);

	// ball particles
	mBallParticles = std::make_unique<ParticleGen>(
		mTextures.get(TextureID::Particle),
		500);

	// setup the world data
	// levels
	auto &blocksTex = mTextures.get(TextureID::Blocks);
	for (auto path : levels)
	{
		Level level;
		if (!loadLevel(path, level))
		{
			std::cerr << "Level '" << path << "' error\n";
			continue;
		}
		level.texture = blocksTex;
		mLevels.push_back(std::move(level));
	}

	// player and ball
	mPlayer.texture = mTextures.get(TextureID::Paddle);
	mBall.size = glm::vec2(BallRadius * 2.f);
	mBall.texture = mTextures.get(TextureID::Face);
	resetPlayer();
}

Game::~Game()
{
	mTextures.destroy();
	mShaders.destroy();
	mFonts.destroy();
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void
Game::run()
{
	// variable-time game loop
	auto currentTime = glfwGetTime();
	while (!glfwWindowShouldClose(mWindow))
	{
		auto newTime = glfwGetTime();
		auto frameTime = newTime - currentTime;
		currentTime = newTime;

		processInput();
		update(frameTime);
		mAudioDevice.update();

		render();
		glfwSwapBuffers(mWindow);
	}
}

void
Game::processInput()
{
	mEventQueue.poll();
	Event event;
	while (mEventQueue.pop(event))
	{
		handleEvent(event);
	}
}

void
Game::handleEvent(const Event &event)
{
	switch (mState)
	{
	case State::Active:
		if (const auto ep(std::get_if<KeyPressed>(&event)); ep)
		{
			switch (ep->key)
			{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(ep->window, GLFW_TRUE);
				break;
			default:
				break;
			}
		}
		break;
	case State::Menu:
		if (const auto ep(std::get_if<KeyPressed>(&event)); ep)
		{
			switch (ep->key)
			{
			case GLFW_KEY_ENTER:
				mState = State::Active;
				break;
			case GLFW_KEY_W:
				mCurrentLevel = (mCurrentLevel + 1) % mLevels.size();
				break;
			case GLFW_KEY_S:
				mCurrentLevel = (mCurrentLevel + mLevels.size()-1) % mLevels.size();
				break;
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(ep->window, GLFW_TRUE);
				break;
			}
		}
		break;
	case State::Win:
		if (const auto ep(std::get_if<KeyPressed>(&event)); ep)
		{
			switch (ep->key)
			{
			case GLFW_KEY_ENTER:
				mEffects->Chaos = false;
				mState = State::Menu;
				break;
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(ep->window, GLFW_TRUE);
				break;
			}
		}
		break;
	}
}

void
Game::update(GLfloat dt)
{
	if (mState != State::Active)
	{
		return;
	}

	// update the paddle
	auto vel = PlayerVelocity * dt;
	if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (mPlayer.pos.x >= 0.f)
		{
			mPlayer.pos -= vel;
			if (mBall.stuck)
			{
				mBall.pos -= vel;
			}
		}
	}

	if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		if (mPlayer.pos.x + mPlayer.size.x < ScreenWidth)
		{
			mPlayer.pos += vel;
			if (mBall.stuck)
			{
				mBall.pos += vel;
			}
		}
	}
	if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		mBall.stuck = false;
	}

	// update the ball
	if (!mBall.stuck)
	{
		mBall.pos += mBall.vel * dt;
		if (mBall.pos.x <= 0.f)
		{
			mBall.pos.x = 0.f;
			mBall.vel.x = -mBall.vel.x;
		}
		else if (mBall.pos.x + mBall.size.x >= ScreenWidth)
		{
			mBall.pos.x = ScreenWidth - mBall.size.x;
			mBall.vel.x = -mBall.vel.x;
		}

		if (mBall.pos.y <= 0.f)
		{
			mBall.pos.y = 0.f;
			mBall.vel.y = -mBall.vel.y;
		}
	}

	// compute the collisions
	doCollisions();

	mBallParticles->update(dt, 2,
	                   mBall.pos + glm::vec2(BallRadius/2.f),
	                   mBall.vel);

	// remove and update the powerups
	std::erase_if(mPowerUPs, [](const auto &p) {
		return p.dead;
	});
	for (auto &pow : mPowerUPs)
	{
		pow.pos += pow.vel * dt;
	}

	// update the effects
	if (!mShakeEffect.update(dt))
	{
		mEffects->Shake = false;
	}
	if (!mStickyEffect.update(dt))
	{
		mPlayer.color = glm::vec3(1.f);
	}
	if (!mPassThroughEffect.update(dt))
	{
		mBall.color = glm::vec3(1.f);
	}
	if (!mConfuseEffect.update(dt))
	{
		mEffects->Confuse = false;
	}
	if (!mChaosEffect.update(dt))
	{
		mEffects->Chaos = false;
	}

	if (mBall.pos.y >= ScreenHeight)
	{
		if (--mLives == 0)
		{
			resetLevel();
			mState = State::Menu;
			mAudioDevice.play(SoundID::Over);
		}
		else
		{
			mAudioDevice.play(SoundID::Dead);
		}
		resetPlayer();
	}

	const auto &bricks = mLevels[mCurrentLevel].blocks;
	if (std::all_of(bricks.begin(), bricks.end(),[](const auto &b) {
		return b.solid||b.dead;
	}))
	{
		resetLevel();
		resetPlayer();
		mEffects->Chaos = true;
		mState = State::Win;
	}
}

void Game::render()
{
	mRenderer->clear(glm::vec4(0.f, 0.f, .2f, 1.f));

	auto &font = mFonts.get(FontID::Title);
	if (mState == State::Active || mState == State::Menu)
	{
		mEffects->beginRender();

		auto background = mTextures.get(TextureID::Background);
		mRenderer->draw(background,
		                glm::vec2(0.0f),
		                glm::vec2(ScreenWidth, ScreenHeight));

		mRenderer->draw(mLevels[mCurrentLevel]);

		mRenderer->draw(mPlayer);

		for (const auto &p : mPowerUPs)
		{
			mRenderer->draw(p);
		}

		mRenderer->draw(*mBallParticles);

		mRenderer->draw(mBall);

		mEffects->endRender();

		mRenderer->draw(*mEffects, glfwGetTime());

		std::stringstream ss;
		ss << "Lives: " << mLives;
		mRenderer->draw(ss.str(), {5.0f, 5.0f}, font);
	}

	if (mState == State::Menu)
	{
		mRenderer->draw("Press ENTER to start", {250.0f, ScreenHeight / 2}, font);

		auto &small = mFonts.get(FontID::Subtitle);
		mRenderer->draw("Press W or S to select level", {245.0f, ScreenHeight/2 + 20.0f}, small);
	}

	if (mState == State::Win)
	{
		mRenderer->draw("You WON!!!",
		                      {320.0f, ScreenHeight / 2 - 20.0f},
		                      font, glm::vec3(0.0f, 1.0f, 0.0f));
		mRenderer->draw("Press ENTER to retry or ESC to quit",
		                     {130.0f, ScreenHeight / 2,},
		                     font, glm::vec3(1.0f, 1.0f, 0.0f));
	}
}

bool
Game::loadLevel(const std::filesystem::path &path, Level &level)
{
	level.blocks.clear();
	std::ifstream input(path);
	if (input.fail())
	{
		std::cerr << "Game::loadLevel() - failed to load \""
		          << path << "\".";
		return false;
	}

	std::vector<std::vector<unsigned>> tileData;
	std::string line;
	while (std::getline(input, line))
	{
		std::istringstream ss(line);
		std::vector<unsigned> row;

		unsigned tileCode;
		while (ss >> tileCode)
		{
			row.push_back(tileCode);
		}

		tileData.push_back(row);
	}

	auto height = tileData.size();
	auto width = tileData[0].size();
	float unit_width = static_cast<float>(ScreenWidth / width);
	float unit_height = static_cast<float>(ScreenHeight / 2 / height);
	level.blockSize = glm::vec2(unit_width, unit_height);

	float offset = static_cast<float>((ScreenWidth % width) / 2);
	glm::vec2 pos(0.f);
	for (decltype(height) y = 0; y < height; ++y, pos.y += unit_height)
	{
		pos.x = offset;
		for (decltype(width) x = 0; x < width; ++x, pos.x += unit_width)
		{
			Block b{pos, tileData[y][x], false, false};

			switch (b.type)
			{
			case 1: b.solid = true;
			case 2:
			case 3:
			case 4:
			case 5: break;
			default: continue; // empty brick
			}
			level.blocks.push_back(b);
		}
	}
	return true;
}

void
Game::resetLevel()
{
	for (auto &block : mLevels[mCurrentLevel].blocks)
	{
		block.dead = false;
	}
	mLives = InitialLives;
}

void
Game::resetPlayer()
{
	mPlayer.size = PlayerSize;
	mPlayer.pos.x = (ScreenWidth - PlayerSize.x) * 0.5f;
	mPlayer.pos.y = ScreenHeight - PlayerSize.y;
	mPlayer.color = glm::vec3(1.0f);

	mBall.pos.x = mPlayer.pos.x + PlayerSize.x * 0.5f - BallRadius;
	mBall.pos.y = mPlayer.pos.y - BallRadius * 2.f;
	mBall.vel = InitialBallVelocity;
	mBall.color = glm::vec3(1.f);
	mBall.stuck = true;

	// remove the powerups
	mPowerUPs.clear();

	// disable the effects
	mShakeEffect.disable();
	mStickyEffect.disable();
	mPassThroughEffect.disable();
	mChaosEffect.disable();
	mConfuseEffect.disable();
	mEffects->Chaos = false;
	mEffects->Confuse = false;
}

void
Game::activatePowerUP(enum PowerUP::Type type)
{
	switch (type)
	{
	case PowerUP::Speed:
		mBall.vel *= 1.2;
		break;
	case PowerUP::Sticky:
		mPlayer.color = glm::vec3(1.0f, 0.5f, 1.0f);
		mStickyEffect.enableFor(20.f);
		break;
	case PowerUP::PassThrough:
		mBall.color = glm::vec3(1.0f, 0.5f, 0.5f);
		mPassThroughEffect.enableFor(10.f);
		break;
	case PowerUP::PadIncrease:
		mPlayer.size.x += 50;
		break;
	case PowerUP::Confuse:
		if (!mChaosEffect.isEnabled())
		{
			mConfuseEffect.enableFor(15.f);
			mEffects->Confuse = true;
		}
		break;
	case PowerUP::Chaos:
		if (!mConfuseEffect.isEnabled())
		{
			mChaosEffect.enableFor(15.f);
			mEffects->Chaos = true;
		}
		break;
	}
}

enum class Direction
{
	Up,
	Right,
	Down,
	Left,
};

static Direction
getDirection(glm::vec2 target)
{
	static const glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),  // UP
		glm::vec2(1.0f, 0.0f),  // RIGHT
		glm::vec2(0.0f, -1.0f), // DOWN
		glm::vec2(-1.0f, 0.0f), // LEFT
	};
	float max = 0.0f;
	int best_match = 0;
	for (int i = 0; i < 4; i++)
	{
		float dot = glm::dot(target, compass[i]);
		if (dot > max)
		{
			max = dot;
			best_match = i;
		}
	}
	return static_cast<Direction>(best_match);
}

typedef std::tuple<bool, Direction, glm::vec2> Collision;

static Collision
checkCollision(const Ball &a, glm::vec2 pos, glm::vec2 size)
{
	glm::vec2 ball_center(a.pos + BallRadius);
	glm::vec2 aabb_half_extents(size.x/2, size.y/2);
	glm::vec2 aabb_center(pos + aabb_half_extents);

	glm::vec2 difference = ball_center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

	glm::vec2 closest = aabb_center + clamped;
	difference = closest - ball_center;
	if (glm::dot(difference, difference) < BallRadius * BallRadius)
	{
		return std::make_tuple(true, getDirection(difference), difference);
	}
	return std::make_tuple(false, Direction::Up, difference);
}

static bool
checkCollision(const Paddle &a, const PowerUP &b)
{
	bool cx =
		a.pos.x + a.size.x >= b.pos.x &&
		b.pos.x + b.size.x >= a.pos.x;

	bool cy =
		a.pos.y + a.size.y >= b.pos.y &&
		b.pos.y + b.size.y >= a.pos.y;

	return cx && cy;
}

void
Game::doCollisions()
{
	// ball bricks collision
	glm::vec2 size = mLevels[mCurrentLevel].blockSize;
	for (auto &obj : mLevels[mCurrentLevel].blocks)
	{
		if (obj.dead)
		{
			continue;
		}

		auto [col, dir, vec] = checkCollision(mBall, obj.position, size);
		if (!col)
		{
			continue;
		}
		if (!obj.solid)
		{
			obj.dead = true;
			spawnPowerUPs(obj.position);
			mAudioDevice.play(SoundID::Block);
		}
		else
		{
			mShakeEffect.enableFor(0.05f);
			mEffects->Shake = true;
			mAudioDevice.play(SoundID::Solid);
		}

		if (mPassThroughEffect.isEnabled() && !obj.solid)
		{
			// nothing
		}
		else if (dir == Direction::Left || dir == Direction::Right)
		{
			mBall.vel.x = -mBall.vel.x;
			float penetration = BallRadius - std::abs(vec.x);
			if (dir == Direction::Left)
			{
				mBall.pos.x += penetration;
			}
			else
			{
				mBall.pos.x -= penetration;
			}
		}
		else
		{
			mBall.vel.y = -mBall.vel.y;
			float penetration = BallRadius - std::abs(vec.y);
			if (dir == Direction::Down)
			{
				mBall.pos.y += penetration;
			}
			else
			{
				mBall.pos.y -= penetration;
			}
		}
	}

	// ball player collision
	if (!mBall.stuck)
	{
		Collision c = checkCollision(mBall, mPlayer.pos, mPlayer.size);
		if (std::get<0>(c))
		{
			float center = mPlayer.pos.x + mPlayer.size.x / 2;
			float distance = mBall.pos.x + BallRadius - center;
			float percentage = distance / (mPlayer.size.x / 2);

			float strength = 2.0f;

			glm::vec2 oldvel = mBall.vel;
			mBall.vel.x = InitialBallVelocity.x * percentage * strength;
			mBall.vel.y = -1.0f * std::abs(mBall.vel.y);
			mBall.vel = glm::normalize(mBall.vel) * glm::length(oldvel);
			mBall.stuck = mStickyEffect.isEnabled();

			mAudioDevice.play(SoundID::Paddle);
		}
	}

	// powerup player collision
	for (PowerUP &p : mPowerUPs)
	{
		if (p.pos.y >= ScreenHeight)
		{
			p.dead= true;
		}
		else if (checkCollision(mPlayer, p))
		{
			activatePowerUP(p.type);
			p.dead = true;
			mAudioDevice.play(SoundID::Powerup);
		}
	}
}

static bool
shouldSpawn(unsigned chance)
{
	return (rand() % chance) == 0;
}

void
Game::spawnPowerUPs(glm::vec2 pos)
{
	PowerUP pow;
	if (shouldSpawn(75))
	{
		pow.type = PowerUP::Speed;
		pow.color = glm::vec3(0.5f, 0.5f, 1.0f);
		pow.texture = mTextures.get(TextureID::PowerupSpeed);
	}
	else if (shouldSpawn(75))
	{
		pow.type = PowerUP::Sticky;
		pow.color = glm::vec3(1.0f, 0.5f, 1.0f);
		pow.texture = mTextures.get(TextureID::PowerupSticky);
	}
	else if (shouldSpawn(75))
	{
		pow.type = PowerUP::PassThrough;
		pow.color = glm::vec3(0.5f, 1.0f, 0.5f);
		pow.texture = mTextures.get(TextureID::PowerupPassthrough);
	}
	else if (shouldSpawn(75))
	{
		pow.type = PowerUP::PadIncrease;
		pow.color = glm::vec3(1.0f, 0.6f, 0.4f);
		pow.texture = mTextures.get(TextureID::PowerupIncrease);
	}
	else if (shouldSpawn(15))
	{
		pow.type = PowerUP::Confuse;
		pow.color = glm::vec3(1.0f, 0.3f, 0.3f);
		pow.texture = mTextures.get(TextureID::PowerupConfuse);
	}
	else if (shouldSpawn(15))
	{
		pow.type = PowerUP::Chaos;
		pow.color = glm::vec3(0.9f, 0.25f, 0.25f);
		pow.texture = mTextures.get(TextureID::PowerupChaos);
	}
	else
	{
		return;
	}
	pow.pos = pos;
	pow.size = PowerUPSize;
	pow.vel = PowerUPVelocity;
	pow.dead = false;

	mPowerUPs.push_back(pow);
}

void
Game::loadAssets()
{
        // textures
	static constexpr std::pair<TextureID, std::string_view> textures[] = {
		{ TextureID::Face, "assets/textures/awesomeface.png" },
		{ TextureID::Background, "assets/textures/background.jpg" },
		{ TextureID::Blocks, "assets/textures/blocks.png" },
		{ TextureID::Paddle, "assets/textures/paddle.png" },
		{ TextureID::Particle, "assets/textures/particle.png" },
		{ TextureID::PowerupSpeed, "assets/textures/powerup_speed.png" },
		{ TextureID::PowerupSticky, "assets/textures/powerup_sticky.png" },
		{ TextureID::PowerupIncrease, "assets/textures/powerup_increase.png" },
		{ TextureID::PowerupConfuse, "assets/textures/powerup_confuse.png" },
		{ TextureID::PowerupChaos, "assets/textures/powerup_chaos.png" },
		{ TextureID::PowerupPassthrough, "assets/textures/powerup_passthrough.png" },
	};
	for (auto [id, path] : textures)
	{
		mTextures.load(id, path);
	}

	// shaders
	static constexpr std::tuple<ShaderID, std::string_view, std::string_view> shaders[] = {
		{ ShaderID::Postprocess, "assets/shaders/postprocess.vs", "assets/shaders/postprocess.fs" },
		{ ShaderID::Texture, "assets/shaders/simple.vs", "assets/shaders/texture.fs" },
		{ ShaderID::UniformColor, "assets/shaders/simple.vs", "assets/shaders/uniformcolor.fs" },
		{ ShaderID::VertexColor, "assets/shaders/vertexcolor.vs", "assets/shaders/vertexcolor.fs" },
	};
	for (auto [id, vs, fs] : shaders)
	{
		mShaders.load(id, vs, fs);
	}

	// fonts
	static constexpr std::tuple<FontID, std::string_view, unsigned> fonts[] = {
		{ FontID::Title, "assets/fonts/ocraext.ttf", 24 },
		{ FontID::Subtitle, "assets/fonts/ocraext.ttf", 18 },
	};
	for (auto [id, path, size] : fonts)
	{
		mFonts.load(id, path, size);
	}

	// sound buffers
	static constexpr std::pair<SoundID, std::string_view> sounds[] = {
		{ SoundID::Block,   "assets/audio/block.wav"   },
		{ SoundID::Dead,    "assets/audio/dead.wav"    },
		{ SoundID::Over,    "assets/audio/over.wav"    },
		{ SoundID::Paddle,  "assets/audio/paddle.wav"  },
		{ SoundID::Powerup, "assets/audio/powerup.wav" },
		{ SoundID::Solid,   "assets/audio/solid.wav"   },
	};
	for (auto [id, path] : sounds)
	{
		mAudioDevice.load(id, path);
	}
}
