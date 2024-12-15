#pragma once

#include <variant>
#include <GLFW/glfw3.h>

// list of events

struct WindowResized
{
	GLFWwindow *window;
	int width;
	int height;
};

struct KeyPressed
{
	GLFWwindow *window;
	int key;
	int scancode;
	int mod;
};

struct KeyReleased
{
	GLFWwindow *window;
	int key;
	int scancode;
	int mod;
};

struct KeyRepeated
{
	GLFWwindow *window;
	int key;
	int scancode;
	int mod;
};

struct MouseButtonPressed
{
	GLFWwindow *window;
	int button;
	int mods;
};

struct MouseButtonReleased
{
	GLFWwindow *window;
	int button;
	int mods;
};

struct MouseCursorMoved
{
	GLFWwindow *window;
	double x;
	double y;
};

using Event = std::variant<WindowResized,
                           KeyPressed,
                           KeyRepeated,
                           KeyReleased,
                           MouseButtonPressed,
                           MouseButtonReleased,
                           MouseCursorMoved>;
