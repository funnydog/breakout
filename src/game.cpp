#include <cstdint>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

#include "ball.hpp"
#include "font.hpp"
#include "game.hpp"
#include "glcheck.hpp"
#include "particle.hpp"
#include "postprocess.hpp"

namespace
{
static constexpr glm::vec2 PlayerSize(100, 20);
static constexpr glm::vec2 PlayerVelocity(500.f, 0.f);
static constexpr glm::vec2 InitialBallVelocity(100.0f, -350.0f);
static constexpr float BallRadius = 12.5f;

static constexpr unsigned InitialLives = 3;

static const char *levels[] = {
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
	, mShakeTime(0.0f)
	, mCurrentLevel(0)
	, mLives(InitialLives)
	, mWindow(nullptr)
	, mKeys()
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

	mEventQueue.track(mWindow);

	// load the assets
	loadAssets();

	// create the orthographic projection matrix
	glm::mat4 proj = glm::ortho(
		0.0f, static_cast<GLfloat>(ScreenWidth),
		static_cast<GLfloat>(ScreenHeight), 0.0f,
		-1.0f, 1.0f);

	// configure the sprite shader
	auto spriteShader = mShaders.get(ShaderID::Sprite);
	spriteShader.use();
	spriteShader.getUniform("projection").setMatrix4(proj);

	// configure the text shader
	auto textShader = mShaders.get(ShaderID::Text);
	textShader.use();
	textShader.getUniform("projection").setMatrix4(proj);

	// configure the particle shader
	auto particleShader = mShaders.get(ShaderID::Particle);
	particleShader.use();
	particleShader.getUniform("sprite").setInteger(0);
	particleShader.getUniform("projection").setMatrix4(proj);

	// configure the level shader
	auto levelShader = mShaders.get(ShaderID::Blocks);
	levelShader.use();
	levelShader.getUniform("image").setInteger(0);
	levelShader.getUniform("projection").setMatrix4(proj);

	// make the batch renderer
	mRenderer = std::make_unique<Renderer>(
		textShader,
		levelShader,
		particleShader,
		spriteShader);

	// set-up the effects
	mEffects = std::make_unique<Postprocess>(
		mShaders.get(ShaderID::Postprocess),
		ScreenWidth,
		ScreenHeight);

	// levels
	auto &blocksTex = mTextures.get(TextureID::Blocks);
	for (const char *path : levels)
	{
		GameLevel level;
		if (!level.load(path, blocksTex, ScreenWidth, ScreenHeight / 2))
		{
			std::cerr << "Level '" << path << "' error\n";
			continue;
		}
		mLevels.push_back(level);
	}

	// player
	glm::vec2 playerPos = glm::vec2(
		ScreenWidth / 2 - PlayerSize.x / 2,
		ScreenHeight - PlayerSize.y);

	mPlayer = std::make_unique<GameObject>(
		playerPos, PlayerSize,
		mTextures.get(TextureID::Paddle));

	// ball
	glm::vec2 ballPos = playerPos + glm::vec2(
		PlayerSize.x / 2 - BallRadius,
		-BallRadius * 2);

	mBall = std::make_unique<BallObject>(
		ballPos, BallRadius, InitialBallVelocity,
		mTextures.get(TextureID::Face));

	// ball particles
	mBallParticles = std::make_unique<ParticleGen>(
		mTextures.get(TextureID::Particle),
		500);
}

Game::~Game()
{
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void
Game::run()
{
	auto currentTime = glfwGetTime();
	while (!glfwWindowShouldClose(mWindow))
	{
		auto newTime = glfwGetTime();
		auto frameTime = newTime - currentTime;
		currentTime = newTime;

		processInput();
		update(frameTime);

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
				mKeys[ep->key] = GL_TRUE;
				break;
			}
		}
		else if (const auto ep(std::get_if<KeyReleased>(&event)); ep)
		{
			mKeys[ep->key] = GL_FALSE;
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
	if (mKeys[GLFW_KEY_A])
	{
		if (mPlayer->Position.x >= 0.f)
		{
			mPlayer->Position -= vel;
			if (mBall->Stuck)
			{
				mBall->Position -= vel;
			}
		}
	}

	if (mKeys[GLFW_KEY_D])
	{
		if (mPlayer->Position.x + mPlayer->Size.x < ScreenWidth)
		{
			mPlayer->Position += vel;
			if (mBall->Stuck)
			{
				mBall->Position += vel;
			}
		}
	}
	if (mKeys[GLFW_KEY_SPACE])
	{
		mBall->Stuck = false;
	}

	// update the ball
	mBall->Move(dt, ScreenWidth);
	doCollisions();

	mBallParticles->update(dt, 2,
	                   mBall->Position + glm::vec2(mBall->Radius/2.f),
	                   mBall->Velocity);

	updatePowerUPs(dt);

	if (mShakeTime > 0.f)
	{
		mShakeTime -= dt;
		if (mShakeTime <= 0.0f)
		{
			mEffects->Shake = false;
		}
	}

	if (mBall->Position.y >= ScreenHeight)
	{
		if (--mLives == 0)
		{
			resetLevel();
			mState = State::Menu;
		}
		resetPlayer();
	}

	if(mLevels[mCurrentLevel].isCompleted())
	{
		resetLevel();
		resetPlayer();
		mEffects->Chaos = true;
		mState = State::Win;
	}
}

void Game::render()
{
	auto &font = mFonts.get(FontID::Title);
	if (mState == State::Active || mState == State::Menu)
	{
		mEffects->BeginRender();

		auto background = mTextures.get(TextureID::Background);
		mRenderer->draw(background,
		                glm::vec2(0.0f),
		                glm::vec2(ScreenWidth, ScreenHeight));

		mRenderer->draw(mLevels[mCurrentLevel]);

		mRenderer->draw(mPlayer->Sprite, mPlayer->Position, mPlayer->Size, mPlayer->Color);

		for (PowerUP &p : mPowerUPs)
		{
			if (!p.Destroyed)
			{
				mRenderer->draw(p.Sprite, p.Position, p.Size, p.Color);
			}
		}

		mRenderer->draw(*mBallParticles);

		mRenderer->draw(mBall->Sprite, mBall->Position, mBall->Size, mBall->Color);

		mEffects->EndRender();
		mEffects->Render(glfwGetTime());

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

void
Game::resetLevel()
{
	mLevels[mCurrentLevel].reset();
	mLives = InitialLives;
}

void
Game::resetPlayer()
{
	mPlayer->Size = PlayerSize;
	mPlayer->Position = glm::vec2(
		ScreenWidth / 2 - PlayerSize.x / 2,
		ScreenHeight - PlayerSize.y);
	mPlayer->Color = glm::vec3(1.0f);

	mBall->Reset(
		mPlayer->Position + glm::vec2(
			PlayerSize.x / 2 - BallRadius,
			-BallRadius * 2),
		InitialBallVelocity);
	mBall->PassThrough = false;
	mBall->Sticky = false;
	mBall->Color = glm::vec3(1.0f);

	mEffects->Chaos = false;
	mEffects->Confuse = false;
}

void
Game::activatePowerUP(PowerUP &p)
{
	switch (p.Type)
	{
	case PowerUP::SPEED:
		mBall->Velocity *= 1.2;
		break;
	case PowerUP::STICKY:
		mBall->Sticky = true;
		mPlayer->Color = glm::vec3(1.0f, 0.5f, 1.0f);
		break;
	case PowerUP::PASSTHROUGH:
		mBall->PassThrough = true;
		mBall->Color = glm::vec3(1.0f, 0.5f, 0.5f);
		break;
	case PowerUP::PAD_INCREASE:
		mPlayer->Size.x += 50;
		break;
	case PowerUP::CONFUSE:
		if (!mEffects->Chaos)
		{
			mEffects->Confuse = true;
		}
		break;
	case PowerUP::CHAOS:
		if (!mEffects->Confuse)
		{
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
checkCollision(const BallObject &a, glm::vec2 pos, glm::vec2 size)
{
	glm::vec2 ball_center(a.Position + a.Radius);
	glm::vec2 aabb_half_extents(size.x/2, size.y/2);
	glm::vec2 aabb_center(pos + aabb_half_extents);

	glm::vec2 difference = ball_center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

	glm::vec2 closest = aabb_center + clamped;
	difference = closest - ball_center;
	if (glm::dot(difference, difference) < a.Radius * a.Radius)
	{
		return std::make_tuple(true, getDirection(difference), difference);
	}
	return std::make_tuple(false, Direction::Up, difference);
}

static bool
checkCollision(const GameObject &a, const GameObject &b)
{
	bool cx =
		a.Position.x + a.Size.x >= b.Position.x &&
		b.Position.x + b.Size.x >= a.Position.x;

	bool cy =
		a.Position.y + a.Size.y >= b.Position.y &&
		b.Position.y + b.Size.y >= a.Position.y;

	return cx && cy;
}

void
Game::doCollisions()
{
	// ball bricks collision
	glm::vec2 size = mLevels[mCurrentLevel].getBrickSize();
	for (auto &obj : mLevels[mCurrentLevel].mBricks)
	{
		if (obj.dead)
		{
			continue;
		}

		auto [col, dir, vec] = checkCollision(*mBall, obj.position, size);
		if (!col)
		{
			continue;
		}
		if (!obj.solid)
		{
			obj.dead = true;
			spawnPowerUPs(obj.position);
		}
		else
		{
			mShakeTime = 0.05f;
			mEffects->Shake = true;
		}

		if (mBall->PassThrough && !obj.solid)
		{
			// nothing
		}
		else if (dir == Direction::Left || dir == Direction::Right)
		{
			mBall->Velocity.x = -mBall->Velocity.x;
			float penetration = mBall->Radius - std::abs(vec.x);
			if (dir == Direction::Left)
			{
				mBall->Position.x += penetration;
			}
			else
			{
				mBall->Position.x -= penetration;
			}
		}
		else
		{
			mBall->Velocity.y = -mBall->Velocity.y;
			float penetration = mBall->Radius - std::abs(vec.y);
			if (dir == Direction::Down)
			{
				mBall->Position.y += penetration;
			}
			else
			{
				mBall->Position.y -= penetration;
			}
		}
	}

	// ball player collision
	if (!mBall->Stuck)
	{
		Collision c = checkCollision(*mBall, mPlayer->Position, mPlayer->Size);
		if (std::get<0>(c))
		{
			float center = mPlayer->Position.x + mPlayer->Size.x / 2;
			float distance = mBall->Position.x + mBall->Radius - center;
			float percentage = distance / (mPlayer->Size.x / 2);

			float strength = 2.0f;

			glm::vec2 oldVelocity = mBall->Velocity;
			mBall->Velocity.x = InitialBallVelocity.x * percentage * strength;
			mBall->Velocity.y = -1.0f * std::abs(mBall->Velocity.y);
			mBall->Velocity = glm::normalize(mBall->Velocity) * glm::length(oldVelocity);
			mBall->Stuck = mBall->Sticky;
		}
	}

	// powerup player collision
	for (PowerUP &p : mPowerUPs)
	{
		if (p.Destroyed)
		{
			// nothing
		}
		else if (p.Position.y >= ScreenHeight)
		{
			p.Destroyed = true;
		}
		else if (checkCollision(*mPlayer, p))
		{
			activatePowerUP(p);
			p.Destroyed = true;
			p.Activated = true;
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
	if (shouldSpawn(75))
	{
		mPowerUPs.emplace_back(
			PowerUP::SPEED, glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, pos,
			mTextures.get(TextureID::PowerupSpeed));
	}
	else if (shouldSpawn(75))
	{
		mPowerUPs.emplace_back(
			PowerUP::STICKY, glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, pos,
			mTextures.get(TextureID::PowerupSticky));
	}
	else if (shouldSpawn(75))
	{
		mPowerUPs.emplace_back(
			PowerUP::PASSTHROUGH, glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, pos,
			mTextures.get(TextureID::PowerupPassthrough));
	}
	else if (shouldSpawn(75))
	{
		mPowerUPs.emplace_back(
			PowerUP::PAD_INCREASE, glm::vec3(1.0f, 0.6f, 0.4f), 0.0f, pos,
			mTextures.get(TextureID::PowerupIncrease));
	}
	else if (shouldSpawn(15))
	{
		mPowerUPs.emplace_back(
			PowerUP::CONFUSE, glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, pos,
			mTextures.get(TextureID::PowerupConfuse));
	}
	else if (shouldSpawn(15))
	{
		mPowerUPs.emplace_back(
			PowerUP::CHAOS, glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, pos,
			mTextures.get(TextureID::PowerupChaos));
	}
}

static bool
isOtherPowerUPActive(const std::vector<PowerUP> &powerups, enum PowerUP::Type type)
{
	return std::any_of(powerups.begin(), powerups.end(), [type](const auto &p) {
		return p.Activated && p.Type == type;
	});
}

void
Game::updatePowerUPs(float dt)
{
	for (PowerUP &p : mPowerUPs)
	{
		p.Position += p.Velocity * dt;
		if (!p.Activated)
		{
			continue;
		}

		p.Duration -= dt;
		if (p.Duration > 0.0f)
		{
			continue;
		}

		p.Activated = false;
		switch (p.Type)
		{
		case PowerUP::STICKY:
			if (!isOtherPowerUPActive(mPowerUPs, p.Type))
			{
				mBall->Sticky = false;
				mPlayer->Color = glm::vec3(1.0f);
			}
			break;
		case PowerUP::PASSTHROUGH:
			if (!isOtherPowerUPActive(mPowerUPs, p.Type))
			{
				mBall->PassThrough = false;
				mBall->Color = glm::vec3(1.0f);
			}
			break;
		case PowerUP::CONFUSE:
			if (!isOtherPowerUPActive(mPowerUPs, p.Type))
			{
				mEffects->Confuse = false;
			}
			break;
		case PowerUP::CHAOS:
			if (!isOtherPowerUPActive(mPowerUPs, p.Type))
			{
				mEffects->Chaos = false;
			}
			break;
		default:
			break;
		}
	}
	mPowerUPs.erase(std::remove_if(mPowerUPs.begin(),
	                               mPowerUPs.end(),
	                               [](const auto &p)
	                               {
		                               return p.Destroyed && !p.Activated;
	                               }),
	                mPowerUPs.end());
}

void
Game::loadAssets()
{
        // textures
	static const std::pair<TextureID, std::filesystem::path> textures[] = {
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

	for (auto &[id, path] : textures)
	{
		mTextures.load(id, path);
	}

	// shaders
	mShaders.load(ShaderID::Sprite,
	              "assets/shaders/sprite.vs",
	              "assets/shaders/sprite.fs");
	mShaders.load(ShaderID::Text,
	              "assets/shaders/text.vs",
	              "assets/shaders/text.fs");
	mShaders.load(ShaderID::Particle,
	              "assets/shaders/particle.vs",
	              "assets/shaders/particle.fs");
	mShaders.load(ShaderID::Postprocess,
	              "assets/shaders/postprocess.vs",
	              "assets/shaders/postprocess.fs");
	mShaders.load(ShaderID::Blocks,
	              "assets/shaders/blocks.vs",
	              "assets/shaders/blocks.fs");

	// fonts
	mFonts.load(FontID::Title, "assets/fonts/ocraext.ttf", 24);
	mFonts.load(FontID::Subtitle, "assets/fonts/ocraext.ttf", 18);
}
