#pragma once

#include <vector>

#include "event.hpp"

class EventQueue
{
public:
	EventQueue();
	~EventQueue();

	void poll();
	bool empty() const noexcept;
	bool pop(Event &event) noexcept;
	void track(GLFWwindow *window);

private:
	void add(Event&& event);

	static EventQueue& getEventQueue(GLFWwindow *window);

	static void windowSizeCallback(GLFWwindow *window, int w, int h);
	static void windowClosedCallback(GLFWwindow *window);
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	static void cursorPosCallback(GLFWwindow *window, double x, double y);

private:
	std::vector<GLFWwindow *> mWindows;

	static constexpr std::size_t QUEUE_SIZE = 256;
	Event mEvents[QUEUE_SIZE];
	unsigned mRead;
	unsigned mWrite;
};
