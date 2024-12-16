#include <cassert>
#include <iostream>

#include "eventqueue.hpp"

EventQueue::EventQueue()
	: mWindows(), mEvents(), mRead(0), mWrite(0)
{
}

EventQueue::~EventQueue()
{
	// unregister the callbacks
	for (auto wptr : mWindows)
	{
		glfwSetWindowSizeCallback(wptr, nullptr);
		glfwSetKeyCallback(wptr, nullptr);
		glfwSetMouseButtonCallback(wptr, nullptr);
		glfwSetCursorPosCallback(wptr, nullptr);
	}
}

void
EventQueue::track(GLFWwindow *window)
{
	assert(window && "window cannot be null");

	glfwSetWindowUserPointer(window, this);
	mWindows.push_back(window);
	glfwSetWindowSizeCallback(window, windowSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
}

void
EventQueue::poll()
{
	glfwPollEvents();
}

bool
EventQueue::empty() const noexcept
{
	return mRead == mWrite;
}

bool
EventQueue::pop(Event& event) noexcept
{
	if (mRead == mWrite)
	{
		return false;
	}

	event = mEvents[mRead & (QUEUE_SIZE-1)];
	++mRead;

	return true;
}

void
EventQueue::add(Event&& event)
{
	if (mRead + QUEUE_SIZE == mWrite)
	{
		std::cerr << "EventQueue::add() failed - queue full.\n";
	}
	else
	{
		mEvents[mWrite & (QUEUE_SIZE-1)] = std::move(event);
		++mWrite;
	}
}

EventQueue&
EventQueue::getEventQueue(GLFWwindow *window)
{
	return *reinterpret_cast<EventQueue*>(glfwGetWindowUserPointer(window));
}

void
EventQueue::windowSizeCallback(GLFWwindow *window, int width, int height)
{
	auto &queue = getEventQueue(window);
	queue.add(WindowResized{window, width, height});
}

void
EventQueue::windowClosedCallback(GLFWwindow *window)
{
	auto &queue = getEventQueue(window);
	queue.add(WindowClosed{window});
}

void
EventQueue::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	auto &queue = getEventQueue(window);
	switch (action)
	{
	case GLFW_PRESS:
		queue.add(KeyPressed{window, key, scancode, mods});
		break;
	case GLFW_RELEASE:
		queue.add(KeyReleased{window, key, scancode, mods});
		break;
	case GLFW_REPEAT:
		queue.add(KeyRepeated{window, key, scancode, mods});
		break;
	default:
		break;
	}
}


void
EventQueue::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	auto &queue = getEventQueue(window);
	switch (action)
	{
	case GLFW_PRESS:
		queue.add(MouseButtonPressed{window, button, mods});
		break;
	case GLFW_RELEASE:
		queue.add(MouseButtonReleased{window, button, mods});
		break;
	default:
		break;
	}
}

void
EventQueue::cursorPosCallback(GLFWwindow *window, double x, double y)
{
	auto &queue = getEventQueue(window);
	queue.add(MouseCursorMoved{window, x, y});
}
