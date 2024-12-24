#include <iostream>

#include "alcheck.hpp"

void
alCheckError(const std::filesystem::path &file, unsigned line, std::string_view expr)
{
	ALenum errorCode = alGetError();
	const char *error = "Unknown error";
	const char *description = "No description";
	switch (errorCode)
	{
	case AL_NO_ERROR:
		return;
	case AL_INVALID_NAME:
		error = "AL_INVALID_NAME";
		description = "A bad name (ID) has been specified.";
		break;
	case AL_INVALID_ENUM:
		error = "AL_INVALID_ENUM";
		description = "An unacceptable value has been specified for an enumerated argument.";
		break;
	case AL_INVALID_VALUE:
		error = "AL_INVALID_VALUE";
		description = "A numeric argument is out of range.";
		break;
	case AL_INVALID_OPERATION:
		error = "AL_INVALID_OPERATION";
		description = "The specified operation is not allowed in the current state.";
		break;
	case AL_OUT_OF_MEMORY:
		error = "AL_OUT_OF_MEMORY";
		description = "There is not enough memory left to execute the command.";
		break;
	default:
		break;
	}

	std::cerr << "An internal OpenAL call failed in " << file.filename()
		  << " (" << line << ")."
		  << "\nExpression: " << expr
		  << "\nError description:\n   " << error
		  << "\n   " << description << "\n\n";
}
