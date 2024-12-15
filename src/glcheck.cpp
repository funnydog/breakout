#include <iostream>
#include <GL/glew.h>

#include "glcheck.hpp"

void glCheckError(const std::filesystem::path &file,
                  unsigned line,
                  std::string_view expression)
{
	const char *err = "unknown error";
	const char *desc = "unknown error description";

	switch (glGetError())
	{
	case GL_NO_ERROR:
		return;
	case GL_INVALID_ENUM:
		err = "GL_INVALID_ENUM";
		desc = "An unacceptable value has been specified for an enumerated argument.";
		break;
	case GL_INVALID_VALUE:
		err = "GL_INVALID_VALUE";
		desc = "A numeric argument is out of range.";
		break;
	case GL_INVALID_OPERATION:
		err = "GL_INVALID_OPERATION";
		desc = "The specified operation is not allowed in the current state.";
		break;
	case GL_STACK_OVERFLOW:
		err = "GL_STACK_OVERFLOW";
		desc = "This command would cause a stack overflow.";
		break;
	case GL_STACK_UNDERFLOW:
		err = "GL_STACK_UNDERFLOW";
		desc = "This command would cause a stack underflow.";
		break;
	case GL_OUT_OF_MEMORY:
		err = "GL_OUT_OF_MEMORY";
		desc = "There is not enough memory left to execute the command.";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		err = "GL_INVALID_FRAMEBUFFER_OPERATION";
		desc = "The object bound to FRAMEBUFFER_BINDING is not complete.";
		break;
	default:
		break;
	}

	std::cerr << "An internal OpenGL call failed in " << file.filename()
	          << "(" << line << ").\nExpression:\n"
	          << expression << "\nError description:\n"
	          << err << "\n" << desc << "\n";
}
