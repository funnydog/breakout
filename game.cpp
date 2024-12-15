#include <algorithm>
#include <iostream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

#include "ball.h"
#include "game.h"
#include "particle.h"
#include "postprocess.h"
#include "resource.h"
#include "sprite.h"
#include "text.h"

static constexpr glm::vec2 PLAYER_SIZE(100, 20);
static constexpr float PLAYER_VELOCITY(500.0f);

static constexpr glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
static constexpr float BALL_RADIUS = 12.5f;

Game::Game(GLuint width, GLuint height) :
	State(State::GAME_MENU), Keys(), KeysProcessed(), Width(width), Height(height), Level(0), Lives(3),
	shakeTime(0.0f)
{
}

Game::~Game()
{
}

void Game::Init()
{
	if (ResourceManager::LoadTexture("face", "textures/awesomeface.png", GL_TRUE) < 0
	    || ResourceManager::LoadTexture("background", "textures/background.jpg", GL_FALSE) < 0
	    || ResourceManager::LoadTexture("block", "textures/block.png", GL_FALSE) < 0
	    || ResourceManager::LoadTexture("block_solid", "textures/block_solid.png", GL_FALSE) < 0
	    || ResourceManager::LoadTexture("paddle", "textures/paddle.png", GL_TRUE) < 0
	    || ResourceManager::LoadTexture("particle", "textures/particle.png", GL_TRUE) < 0
	    || ResourceManager::LoadTexture("powerup_speed", "textures/powerup_speed.png", GL_TRUE) < 0
	    || ResourceManager::LoadTexture("powerup_sticky", "textures/powerup_sticky.png", GL_TRUE) < 0
	    || ResourceManager::LoadTexture("powerup_increase", "textures/powerup_increase.png", GL_TRUE) < 0
	    || ResourceManager::LoadTexture("powerup_confuse", "textures/powerup_confuse.png", GL_TRUE) < 0
	    || ResourceManager::LoadTexture("powerup_chaos", "textures/powerup_chaos.png", GL_TRUE) < 0
	    || ResourceManager::LoadTexture("powerup_passthrough", "textures/powerup_passthrough.png", GL_TRUE) < 0)
		std::cerr << "LoadTexture error\n";

	if (ResourceManager::LoadShader("sprite", "shaders/sprite.vs", "shaders/sprite.fs", nullptr) < 0
	    || ResourceManager::LoadShader("particle", "shaders/particle.vs", "shaders/particle.fs", nullptr) < 0
	    || ResourceManager::LoadShader("effects", "shaders/postprocess.vs", "shaders/postprocess.fs", nullptr) < 0
	    || ResourceManager::LoadShader("text", "shaders/text.vs", "shaders/text.fs", nullptr) < 0)
		std::cerr << "LoadShader error\n";

	const char *levels[] = {
		"levels/one.txt",
		"levels/two.txt",
		"levels/three.txt",
		"levels/four.txt",
	};
	for (const char *path : levels) {
		GameLevel level;
		if (level.Load(path, Width, Height / 2) < 0)
			std::cerr << "Level '" << path << "' error\n";
		else
			Levels.push_back(level);
	}

	this->effects = std::make_unique<Postprocess>(
		ResourceManager::GetShader("effects"),
		this->Width,
		this->Height
		);

	// player
	glm::vec2 playerPos = glm::vec2(
		Width / 2 - PLAYER_SIZE.x / 2,
		Height - PLAYER_SIZE.y);

	this->player = std::make_unique<GameObject>(
		playerPos, PLAYER_SIZE,
		ResourceManager::GetTexture("paddle")
		);

	glm::vec2 ballPos = playerPos + glm::vec2(
		PLAYER_SIZE.x / 2 - BALL_RADIUS,
		-BALL_RADIUS * 2
		);

	this->ball = std::make_unique<BallObject>(
		ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
		ResourceManager::GetTexture("face")
		);

	auto &sprite = ResourceManager::GetShader("sprite");
	sprite.Use();
	sprite.GetUniform("image").SetInteger(0);

	glm::mat4 proj = glm::ortho(
		0.0f, static_cast<GLfloat>(this->Width),
		static_cast<GLfloat>(this->Height), 0.0f,
		-1.0f, 1.0f);
	sprite.GetUniform("projection").SetMatrix4(proj);

	this->renderer = std::make_unique<SpriteRenderer>(sprite);

	auto &text = ResourceManager::GetShader("text");
	text.Use();
	text.GetUniform("projection").SetMatrix4(proj);
	this->text = std::make_unique<TextRenderer>(text);
	this->text->Load("fonts/ocraext.ttf", 24);

	auto &particle = ResourceManager::GetShader("particle");
	particle.Use();
	particle.GetUniform("sprite").SetInteger(0);
	particle.GetUniform("projection").SetMatrix4(proj);
	this->particles = std::make_unique<ParticleGen>(
		particle,
		ResourceManager::GetTexture("particle"),
		500
		);
}

void
Game::ResetLevel()
{
	const char *levels[] = {
		"levels/one.txt",
		"levels/two.txt",
		"levels/three.txt",
		"levels/four.txt",
	};
	if (0 <= this->Level && this->Level < 5) {
		GameLevel level;
		if (level.Load(levels[this->Level], Width, Height / 2) < 0)
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
		Width / 2 - PLAYER_SIZE.x / 2,
		Height - PLAYER_SIZE.y);
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
Game::Update(GLfloat dt)
{
	this->ball->Move(dt, this->Width);
	this->DoCollisions();

	this->particles->Update(dt, *this->ball, 2, glm::vec2(this->ball->Radius/2.0f));

	this->UpdatePowerUPs(dt);

	if (this->shakeTime > 0.0f) {
		this->shakeTime -= dt;
		if (this->shakeTime <= 0.0f)
			this->effects->Shake = false;
	}

	if (this->ball->Position.y >= this->Height) {
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

void
Game::ProcessInput(GLfloat dt)
{
	if (this->State == State::GAME_ACTIVE) {
		GLfloat vel = PLAYER_VELOCITY * dt;

		if (Keys[GLFW_KEY_A]) {
			if (this->player->Position.x >= 0) {
				this->player->Position.x -= vel;
				if (this->ball->Stuck)
					this->ball->Position.x -= vel;
			}
		}

		if (Keys[GLFW_KEY_D]) {
			if (this->player->Position.x <= this->Width - this->player->Size.x) {
				this->player->Position.x += vel;
				if (this->ball->Stuck)
					this->ball->Position.x += vel;
			}
		}

		if (Keys[GLFW_KEY_SPACE])
			this->ball->Stuck = false;
	} else if (this->State == State::GAME_MENU) {
		if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER]) {
			this->State = State::GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
		}

		if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W]) {
			this->Level = (this->Level + 1) % 4;
			this->KeysProcessed[GLFW_KEY_W] = GL_TRUE;
		}

		if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S]) {
			if (this->Level > 0)
				--this->Level;
			else
				this->Level = 3;
			this->KeysProcessed[GLFW_KEY_S] = GL_TRUE;
		}
	} else if (this->State == State::GAME_WIN) {
		if (this->Keys[GLFW_KEY_ENTER]) {
			this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
			this->effects->Chaos = false;
			this->State = State::GAME_MENU;
		}
	}
}

void Game::Render()
{
	if (this->State == State::GAME_ACTIVE || this->State == State::GAME_MENU) {
		this->effects->BeginRender();

		auto background = ResourceManager::GetTexture("background");
		this->renderer->DrawSprite(background, glm::vec2(0.0f), glm::vec2(Width, Height), 0.0f);
		Levels[Level].Draw(*this->renderer);
		this->player->Draw(*this->renderer);
		for (PowerUP &p : this->PowerUPs) {
			if (!p.Destroyed)
				p.Draw(*this->renderer);
		}
		this->particles->Draw();
		this->ball->Draw(*this->renderer);

		this->effects->EndRender();
		this->effects->Render(glfwGetTime());

		std::stringstream ss;
		ss << "Lives: " << this->Lives;
		this->text->RenderText(ss.str(), 5.0f, 5.0f, 1.0f);
	}

	if (this->State == State::GAME_MENU) {
		this->text->RenderText("Press ENTER to start", 250.0f, Height / 2, 1.0f);
		this->text->RenderText("Press W or S to select level", 245.0f, Height/2 + 20.0f, 0.75f);
	}

	if (this->State == State::GAME_WIN) {
		this->text->RenderText(
			"You WON!!!", 320.0f, Height / 2 - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f)
			);
		this->text->RenderText(
			"Press ENTER to retry or ESC to quit",
			130.0f, Height / 2, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f)
			);
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
			if (p.Position.y >= this->Height)
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
		this->PowerUPs.push_back(
			PowerUP(PowerUP::SPEED, glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position,
				ResourceManager::GetTexture("powerup_speed")));
	}

	if (shouldSpawn(75)) {
		this->PowerUPs.push_back(
			PowerUP(PowerUP::STICKY, glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position,
				ResourceManager::GetTexture("powerup_sticky")));
	}

	if (shouldSpawn(75)) {
		this->PowerUPs.push_back(
			PowerUP(PowerUP::PASSTHROUGH, glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position,
				ResourceManager::GetTexture("powerup_passthrough")));
	}

	if (shouldSpawn(75)) {
		this->PowerUPs.push_back(
			PowerUP(PowerUP::PAD_INCREASE, glm::vec3(1.0f, 0.6f, 0.4f), 0.0f, block.Position,
				ResourceManager::GetTexture("powerup_increase")));
	}

	if (shouldSpawn(15)) {
		this->PowerUPs.push_back(
			PowerUP(PowerUP::CONFUSE, glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position,
				ResourceManager::GetTexture("powerup_confuse")));
	}

	if (shouldSpawn(15)) {
		this->PowerUPs.push_back(
			PowerUP(PowerUP::CHAOS, glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position,
				ResourceManager::GetTexture("powerup_chaos")));
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
