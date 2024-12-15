#include <cstdlib>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

void
ShaderUniform::SetFloat(GLfloat value) const noexcept
{
	glUniform1f(this->glHandle, value);
}

void
ShaderUniform::SetFloat1fv(const GLfloat *floats, size_t size) const noexcept
{
	glUniform1fv(this->glHandle, size, floats);
}

void
ShaderUniform::SetInteger(GLint value) const noexcept
{
	glUniform1i(this->glHandle, value);
}

void
ShaderUniform::SetInteger1iv(const GLint *ints, size_t size) const noexcept
{
	glUniform1iv(this->glHandle, size, ints);
}

void
ShaderUniform::SetVector2f(GLfloat x, GLfloat y) const noexcept
{
	glUniform2f(this->glHandle, x, y);
}

void
ShaderUniform::SetVector2f(const glm::vec2 &value) const noexcept
{
	glUniform2fv(this->glHandle, 1, glm::value_ptr(value));
}

void
ShaderUniform::SetVector2fv(const GLfloat floats[][2], size_t size) const noexcept
{
	glUniform2fv(this->glHandle, size, floats[0]);
}

void
ShaderUniform::SetVector3f(GLfloat x, GLfloat y, GLfloat z) const noexcept
{
	glUniform3f(this->glHandle, x, y, z);
}

void
ShaderUniform::SetVector3f(const glm::vec3 &value) const noexcept
{
	glUniform3fv(this->glHandle, 1, glm::value_ptr(value));
}

void
ShaderUniform::SetVector3fv(const GLfloat floats[][3], size_t size) const noexcept
{
	glUniform3fv(this->glHandle, size, floats[0]);
}

void
ShaderUniform::SetVector4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) const noexcept
{
	glUniform4f(this->glHandle, x, y, z, w);
}

void
ShaderUniform::SetVector4f(const glm::vec4 &value) const noexcept
{
	glUniform4fv(this->glHandle, 1, glm::value_ptr(value));
}

void
ShaderUniform::SetVector4fv(const GLfloat floats[][4], size_t size) const noexcept
{
	glUniform4fv(this->glHandle, size, floats[0]);
}

void
ShaderUniform::SetMatrix4(const glm::mat4 &value) const noexcept
{
	glUniformMatrix4fv(this->glHandle, 1, GL_FALSE, glm::value_ptr(value));
}

int
Shader::Create()
{
	this->ID = glCreateProgram();
	if (this->ID == 0)
		return -1;

	return 0;
}

void
Shader::Destroy()
{
	glDeleteProgram(this->ID);
	this->ID = 0;
}

void
Shader::Use() const
{
	glUseProgram(ID);
}

int
Shader::Attach(Shader::Type type, const char *source) const
{
	GLenum gltype;
	switch (type) {
	case Type::VERTEX:   gltype = GL_VERTEX_SHADER;   break;
	case Type::FRAGMENT: gltype = GL_FRAGMENT_SHADER; break;
	case Type::GEOMETRY: gltype = GL_GEOMETRY_SHADER; break;
	case Type::COMPUTE:  gltype = GL_COMPUTE_SHADER;  break;
	default:
		return -1;
	}

	GLuint shader = glCreateShader(gltype);
	if (shader == 0)
		return -1;

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	if (checkShaderCompilation(shader) < 0) {
		glDeleteShader(shader);
		return -1;
	}

	glAttachShader(this->ID, shader);
	glDeleteShader(shader);
	return 0;
}

int
Shader::Link() const
{
	glLinkProgram(this->ID);
	return checkProgramLinkage(this->ID);
}

ShaderUniform
Shader::GetUniform(const char *name) const
{
	GLint loc = glGetUniformLocation(this->ID, name);
	if (loc == -1) {
		std::cerr << "SHADER: Cannot find the location of uniform '" << name << "'\n";
		std::abort();
	}
	return ShaderUniform(loc);
}

ShaderAttrib
Shader::GetAttrib(const char *name) const
{
	GLint loc = glGetAttribLocation(this->ID, name);
	if (loc == -1) {
		std::cerr << "SHADER: Cannot find the location of attrib '" << name << "'\n";
		std::abort();
	}
	return ShaderAttrib(loc);
}

int
Shader::checkShaderCompilation(GLuint object)
{
	GLint success;
	GLchar infoLog[1024];
	glGetShaderiv(object, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(object, sizeof(infoLog), NULL, infoLog);
		std::cerr << "| ERROR::SHADER: Compile-time error: SHADER|\n"
			  << "+------------------------------------------+\n"
			  << infoLog << "\n"
			  << "+------------------------------------------+\n";
		return -1;
	}
	return 0;
}

int
Shader::checkProgramLinkage(GLuint object)
{
	GLint success;
	GLchar infoLog[1024];
	glGetProgramiv(object, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(object, sizeof(infoLog), NULL, infoLog);
		std::cerr << "| ERROR::Shader: Link-time error: Type PROGRAM|\n"
			  << "+---------------------------------------------+\n"
			  << infoLog << "\n"
			  << "+---------------------------------------------+\n";
		return -1;
	}
	return 0;
}
