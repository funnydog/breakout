#include "effect.hpp"

void
Effect::enableFor(float duration)
{
	timeLeft += duration;
}

void
Effect::disable()
{
	timeLeft = 0.f;
}

bool
Effect::update(float dt)
{
	if (timeLeft > 0.f)
	{
		timeLeft -= dt;
		if (timeLeft <= 0.f)
		{
			timeLeft = 0.f;
			return false;
		}
	}
	return true;
}

bool
Effect::isEnabled() const
{
	return timeLeft > 0.f;
}
