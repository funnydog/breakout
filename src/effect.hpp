#pragma once

class Effect
{
public:
	void enableFor(float duration);
	void disable();
	bool update(float dt);
	bool isEnabled() const;

private:
	float timeLeft = 0.f;
};
