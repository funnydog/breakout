#include <cstdint>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

#include "ball.hpp"
#include "game.hpp"
#include "glcheck.hpp"
#include "particle.hpp"
#include "postprocess.hpp"
#include "spriterenderer.hpp"
#include "textrenderer.hpp"

namespace
{
static constexpr glm::vec2 PLAYER_SIZE(100, 20);
static constexpr float PLAYER_VELOCITY(500.0f);

static constexpr glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
static constexpr float BALL_RADIUS = 12.5f;

static const char *levels[] = {
	"assets/levels/one.txt",
	"assets/levels/two.txt",
	"assets/levels/three.txt",
	"assets/levels/four.txt",
};

const std::uint64_t targetFPS = 60;
const float SecondsPerFrame = 1.f / targetFPS;
const int MaxStepsPerFrame = 5;
const unsigned ScreenWidth = 800;
const unsigned ScreenHeight = 600;
}

Game::Game()
	: State(State::GAME_MENU), Level(0), Lives(3),
	  shakeTime(0.0f), mWindow(nullptr), mKeys()
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

	mWindow = glfwCreateWindow(ScreenWidth, ScreenHeight, "Breakout", nullptr, nullptr);
	if (!mWindow)
	{
		glfwGetError(&error);
		throw std::runtime_error(error);
	}
	glfwMakeContextCurrent(mWindow);
	glewExperimental = GL_TRUE;
	glCheck(glewInit());

	glViewport(0, 0, ScreenWidth, ScreenHeight);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mEventQueue.track(mWindow);

	// load the assets
	loadAssets();

	// load the levels
	for (const char *path : levels)
	{
		GameLevel level;
		if (level.Load(mTextures, path, ScreenWidth, ScreenHeight / 2) < 0)
		{
			std::cerr << "Level '" << path << "' error\n";
			continue;
		}
		Levels.push_back(level);
	}


	// player, ball and game shader
	glm::vec2 playerPos = glm::vec2(
		ScreenWidth / 2 - PLAYER_SIZE.x / 2,
		ScreenHeight - PLAYER_SIZE.y);

	player = std::make_unique<GameObject>(
		playerPos, PLAYER_SIZE,
		mTextures.get(TextureID::Paddle));

	glm::vec2 ballPos = playerPos + glm::vec2(
		PLAYER_SIZE.x / 2 - BALL_RADIUS,
		-BALL_RADIUS * 2);

	ball = std::make_unique<BallObject>(
		ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
		mTextures.get(TextureID::Face));

	glm::mat4 proj = glm::ortho(
		0.0f, static_cast<GLfloat>(ScreenWidth),
		static_cast<GLfloat>(ScreenHeight), 0.0f,
		-1.0f, 1.0f);

	auto spriteShader = mShaders.get(ShaderID::Sprite);
	spriteShader.use();
	spriteShader.getUniform("projection").setMatrix4(proj);

	// sprite renderer
	renderer = std::make_unique<SpriteRenderer>(spriteShader);

	// text shader and renderer
	auto textShader = mShaders.get(ShaderID::Text);
	textShader.use();
	textShader.getUniform("projection").setMatrix4(proj);
	text = std::make_unique<TextRenderer>(textShader);

	// set-up the effects
	effects = std::make_unique<Postprocess>(
		mShaders.get(ShaderID::Postprocess),
		ScreenWidth,
		ScreenHeight);

	auto particleShader = mShaders.get(ShaderID::Particle);
	particleShader.use();
	particleShader.getUniform("sprite").setInteger(0);
	particleShader.getUniform("projection").setMatrix4(proj);
	particles = std::make_unique<ParticleGen>(
		particleShader,
		mTextures.get(TextureID::Particle),
		500);
}

Game::~Game()
{
	glfwTerminate();
}

void
Game::run()
{
	const std::uint64_t deltaTicks = glfwGetTimerFrequency() / targetFPS;
	std::uint64_t currentTicks = glfwGetTimerValue();
	std::uint64_t accumulator = 0;

	while (!glfwWindowShouldClose(mWindow))
	{
		auto newTicks = glfwGetTimerValue();
		accumulator += newTicks - currentTicks;
		currentTicks = newTicks;

		for (int steps = MaxStepsPerFrame;
		     steps > 0 && accumulator >= deltaTicks;
		     --steps, accumulator -= deltaTicks)
		{
			processInput();
			update(SecondsPerFrame);
		}

		glClearColor(0.f, 0.f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
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
		switch (State)
		{
		case State::GAME_ACTIVE: {
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
		}
		case State::GAME_MENU: {
			if (const auto ep(std::get_if<KeyPressed>(&event)); ep)
			{
				switch (ep->key)
				{
				case GLFW_KEY_ENTER:
					State = State::GAME_ACTIVE;
					break;
				case GLFW_KEY_W:
					Level = (Level + 1) % 4;
					break;
				case GLFW_KEY_S:
					Level = (Level + 3) % 4;
					break;
				case GLFW_KEY_ESCAPE:
					glfwSetWindowShouldClose(ep->window, GLFW_TRUE);
					break;
				}
			}
			break;
		}
		case State::GAME_WIN: {
			if (const auto ep(std::get_if<KeyPressed>(&event)); ep)
			{
				switch (ep->key)
				{
				case GLFW_KEY_ENTER:
					effects->Chaos = false;
					State = State::GAME_MENU;
					break;

				case GLFW_KEY_ESCAPE:
					glfwSetWindowShouldClose(ep->window, GLFW_TRUE);
					break;
				}
			}
			break;
		}
		}
	}
}

void
Game::ResetLevel()
{
	if (0 <= this->Level && this->Level < 5) {
		GameLevel level;
		if (level.Load(mTextures, levels[this->Level],
		               ScreenWidth, ScreenHeight / 2) < 0)
			std::cerr << "Level '" << levels[this->Level] << "' error\n";
		else
			Levels[this->Level] = level;
	}
	this->Lives = 3;
}

void
Game::ResetPlayer()
{
	this->player->Size = PLAYER_SIZE;
	this->player->Position = glm::vec2(
		ScreenWidth / 2 - PLAYER_SIZE.x / 2,
		ScreenHeight - PLAYER_SIZE.y);
	this->player->Color = glm::vec3(1.0f);
	this->ball->Reset(
		this->player->Position + glm::vec2(
			PLAYER_SIZE.x / 2 - BALL_RADIUS,
			-BALL_RADIUS * 2),
		INITIAL_BALL_VELOCITY);
	this->ball->PassThrough = false;
	this->ball->Sticky = false;
	this->ball->Color = glm::vec3(1.0f);
	this->effects->Chaos = false;
	this->effects->Confuse = false;
}

void
Game::update(GLfloat dt)
{
	// update the paddle
	if (State != State::GAME_ACTIVE)
	{
		return;
	}
	GLfloat vel = PLAYER_VELOCITY * dt;
	if (mKeys[GLFW_KEY_A])
	{
		if (player->Position.x >= 0)
		{
			player->Position.x -= vel;
			if (ball->Stuck)
			{
				ball->Position.x -= vel;
			}
		}
	}

	if (mKeys[GLFW_KEY_D])
	{
		if (player->Position.x <= ScreenWidth - this->player->Size.x)
		{
			player->Position.x += vel;
			if (ball->Stuck)
			{
				ball->Position.x += vel;
			}
		}
	}
	if (mKeys[GLFW_KEY_SPACE])
	{
		ball->Stuck = false;
	}

	// update the ball
	this->ball->Move(dt, ScreenWidth);
	this->DoCollisions();

	this->particles->Update(dt, *this->ball, 2, glm::vec2(this->ball->Radius/2.0f));

	this->UpdatePowerUPs(dt);

	if (this->shakeTime > 0.0f) {
		this->shakeTime -= dt;
		if (this->shakeTime <= 0.0f)
			this->effects->Shake = false;
	}

	if (this->ball->Position.y >= ScreenHeight) {
		--this->Lives;
		if (this->Lives == 0) {
			this->ResetLevel();
			this->State = State::GAME_ACTIVE;
		}
		this->ResetPlayer();
	}

	if (this->State == State::GAME_ACTIVE && this->Levels[this->Level].IsCompleted()) {
		this->ResetLevel();
		this->ResetPlayer();
		this->effects->Chaos = GL_TRUE;
		this->State = State::GAME_WIN;
	}
}

void Game::render()
{
	auto &font = mFonts.get(FontID::Title);
	if (this->State == State::GAME_ACTIVE || this->State == State::GAME_MENU) {
		this->effects->BeginRender();

		auto background = mTextures.get(TextureID::Background);
		renderer->draw(background, glm::vec2(0.0f),
		               glm::vec2(ScreenWidth, ScreenHeight),
		               0.0f);
		Levels[Level].Draw(*renderer);
		player->Draw(*renderer);
		for (PowerUP &p : this->PowerUPs) {
			if (!p.Destroyed)
				p.Draw(*renderer);
		}
		particles->Draw();
		ball->Draw(*renderer);

		effects->EndRender();
		effects->Render(glfwGetTime());

		std::stringstream ss;
		ss << "Lives: " << this->Lives;
		text->draw(ss.str(), {5.0f, 5.0f}, font);
	}

	if (State == State::GAME_MENU) {
		text->draw("Press ENTER to start", {250.0f, ScreenHeight / 2}, font);

		auto &small = mFonts.get(FontID::Subtitle);
		text->draw("Press W or S to select level", {245.0f, ScreenHeight/2 + 20.0f}, small);
	}

	if (State == State::GAME_WIN) {
		text->draw("You WON!!!",
		           {320.0f, ScreenHeight / 2 - 20.0f},
		           font, glm::vec3(0.0f, 1.0f, 0.0f));
		text->draw("Press ENTER to retry or ESC to quit",
		           {130.0f, ScreenHeight / 2,},
		           font, glm::vec3(1.0f, 1.0f, 0.0f));
	}
}

enum Direction
{
	UP,
	RIGHT,
	DOWN,
	LEFT,
};

typedef std::tuple<bool, Direction, glm::vec2> Collision;

static Direction VectorDirection(glm::vec2 target)
{
	static const glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f), // UP
		glm::vec2(1.0f, 0.0f), // RIGHT
		glm::vec2(0.0f, -1.0f), // DOWN
		glm::vec2(-1.0f, 0.0f), // LEFT
	};
	float max = 0.0f;
	int best_match = 0;
//	glm::vec2 normal = glm::normalize(target);
	for (int i = 0; i < 4; i++) {
		float dot = glm::dot(target, compass[i]);
		if (dot > max) {
			max = dot;
			best_match = i;
		}
	}
	return (Direction)best_match;
}

static Collision CheckCollision(BallObject &a, GameObject &b)
{
	glm::vec2 ball_center(a.Position + a.Radius);
	glm::vec2 aabb_half_extents(b.Size.x/2, b.Size.y/2);
	glm::vec2 aabb_center(b.Position + aabb_half_extents);

	glm::vec2 difference = ball_center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

	glm::vec2 closest = aabb_center + clamped;
	difference = closest - ball_center;
	if (glm::dot(difference, difference) < a.Radius * a.Radius)
		return std::make_tuple(true, VectorDirection(difference), difference);
	else
		return std::make_tuple(false, UP, difference);
}

static bool CheckCollision(GameObject &a, GameObject &b)
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
Game::ActivatePowerUP(PowerUP &p)
{
	if (p.Type == PowerUP::SPEED) {
		this->ball->Velocity *= 1.2;
	} else if (p.Type == PowerUP::STICKY) {
		this->ball->Sticky = true;
		this->player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	} else if (p.Type == PowerUP::PASSTHROUGH) {
		this->ball->PassThrough = true;
		this->ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
	} else if (p.Type == PowerUP::PAD_INCREASE) {
		this->player->Size.x += 50;
	} else if (p.Type == PowerUP::CONFUSE) {
		if (!this->effects->Chaos)
			this->effects->Confuse = true;
	} else if (p.Type == PowerUP::CHAOS) {
		if (!this->effects->Confuse)
			this->effects->Chaos = true;
	}
}

void
Game::DoCollisions()
{
	// bricks collisions
	for (GameObject &obj : this->Levels[this->Level].Bricks) {
		if (!obj.Destroyed) {
			Collision c = CheckCollision(*this->ball, obj);
			if (std::get<0>(c)) {
				if (!obj.IsSolid) {
					obj.Destroyed = true;
					this->SpawnPowerUPs(obj);
				} else {
					this->shakeTime = 0.05f;
					this->effects->Shake = true;
				}

				Direction dir = std::get<1>(c);
				glm::vec2 difference = std::get<2>(c);
				if ((this->ball->PassThrough && !obj.IsSolid)) {
					// nothing
				} else if (dir == LEFT || dir == RIGHT) {
					this->ball->Velocity.x = -this->ball->Velocity.x;
					float penetration = this->ball->Radius - std::abs(difference.x);
					if (dir == LEFT)
						this->ball->Position.x += penetration;
					else
						this->ball->Position.x -= penetration;
				} else {
					this->ball->Velocity.y = -this->ball->Velocity.y;
					float penetration = this->ball->Radius - std::abs(difference.y);
					if (dir == UP)
						this->ball->Position.y -= penetration;
					else
						this->ball->Position.y += penetration;
				}
			}
		}
	}

	// this->player collision
	if (!this->ball->Stuck) {
		Collision c = CheckCollision(*this->ball, *this->player);
		if (std::get<0>(c)) {
			float center = this->player->Position.x + this->player->Size.x / 2;
			float distance = this->ball->Position.x + this->ball->Radius - center;
			float percentage = distance / (this->player->Size.x / 2);

			float strength = 2.0f;

			glm::vec2 oldVelocity = this->ball->Velocity;
			this->ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
			this->ball->Velocity.y = -1.0f * std::abs(this->ball->Velocity.y);
			this->ball->Velocity = glm::normalize(this->ball->Velocity) * glm::length(oldVelocity);
			this->ball->Stuck = this->ball->Sticky;
		}
	}

	// Powerup collisions
	for (PowerUP &p : this->PowerUPs) {
		if (!p.Destroyed) {
			if (p.Position.y >= ScreenHeight)
				p.Destroyed = true;

			if (CheckCollision(*this->player, p)) {
				ActivatePowerUP(p);
				p.Destroyed = true;
				p.Activated = true;
			}
		}
	}
}

static bool shouldSpawn(unsigned chance)
{
	return (rand() % chance) == 0;
}

void
Game::SpawnPowerUPs(GameObject &block)
{
	if (shouldSpawn(75)) {
		PowerUPs.emplace_back(
			PowerUP::SPEED, glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position,
			mTextures.get(TextureID::PowerupSpeed));
	}

	if (shouldSpawn(75)) {
		PowerUPs.emplace_back(
			PowerUP::STICKY, glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position,
			mTextures.get(TextureID::PowerupSticky));
	}

	if (shouldSpawn(75)) {
		PowerUPs.emplace_back(
			PowerUP::PASSTHROUGH, glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position,
			mTextures.get(TextureID::PowerupPassthrough));
	}

	if (shouldSpawn(75)) {
		PowerUPs.emplace_back(
			PowerUP::PAD_INCREASE, glm::vec3(1.0f, 0.6f, 0.4f), 0.0f, block.Position,
			mTextures.get(TextureID::PowerupIncrease));
	}

	if (shouldSpawn(15)) {
		PowerUPs.emplace_back(
			PowerUP::CONFUSE, glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position,
			mTextures.get(TextureID::PowerupConfuse));
	}

	if (shouldSpawn(15)) {
		PowerUPs.emplace_back(
			PowerUP::CHAOS, glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position,
			mTextures.get(TextureID::PowerupChaos));
	}
}

static bool isOtherPowerUPActive(std::vector<PowerUP> &powerups, enum PowerUP::Type type)
{
	for (const PowerUP &p : powerups) {
		if (p.Activated && p.Type == type)
			return true;
	}
	return false;
}

void
Game::UpdatePowerUPs(float dt)
{
	for (PowerUP &p : this->PowerUPs) {
		p.Position += p.Velocity * dt;
		if (p.Activated) {
			p.Duration -= dt;
			if (p.Duration <= 0.0f) {
				p.Activated = false;
				if (p.Type == PowerUP::STICKY) {
					if (!isOtherPowerUPActive(this->PowerUPs, p.Type)) {
						this->ball->Sticky = false;
						this->player->Color = glm::vec3(1.0f);
					}
				} else if (p.Type == PowerUP::PASSTHROUGH) {
					if (!isOtherPowerUPActive(this->PowerUPs, p.Type)) {
						this->ball->PassThrough = false;
						this->ball->Color = glm::vec3(1.0f);
					}
				} else if (p.Type == PowerUP::CONFUSE) {
					if (!isOtherPowerUPActive(this->PowerUPs, p.Type)) {
						this->effects->Confuse = false;
					}
				} else if (p.Type == PowerUP::CHAOS) {
					if (!isOtherPowerUPActive(this->PowerUPs, p.Type)) {
						this->effects->Chaos = false;
					}
				}
			}
		}
	}
	this->PowerUPs.erase(std::remove_if(this->PowerUPs.begin(), this->PowerUPs.end(),
					    [](const PowerUP &p) { return p.Destroyed && !p.Activated; }
				     ), this->PowerUPs.end());
}

void
Game::loadAssets()
{
        // textures
	mTextures.load(TextureID::Face, "assets/textures/awesomeface.png");
	mTextures.load(TextureID::Background, "assets/textures/background.jpg");
	mTextures.load(TextureID::Block, "assets/textures/block.png");
	mTextures.load(TextureID::BlockSolid, "assets/textures/block_solid.png");
	mTextures.load(TextureID::Paddle, "assets/textures/paddle.png");
	mTextures.load(TextureID::Particle, "assets/textures/particle.png");
	mTextures.load(TextureID::PowerupSpeed, "assets/textures/powerup_speed.png");
	mTextures.load(TextureID::PowerupSticky, "assets/textures/powerup_sticky.png");
	mTextures.load(TextureID::PowerupIncrease, "assets/textures/powerup_increase.png");
	mTextures.load(TextureID::PowerupConfuse, "assets/textures/powerup_confuse.png");
	mTextures.load(TextureID::PowerupChaos, "assets/textures/powerup_chaos.png");
	mTextures.load(TextureID::PowerupPassthrough, "assets/textures/powerup_passthrough.png");

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

	// fonts
	mFonts.load(FontID::Title, "assets/fonts/ocraext.ttf", 24);
	mFonts.load(FontID::Subtitle, "assets/fonts/ocraext.ttf", 18);
}
