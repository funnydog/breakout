#pragma once

#include <filesystem>

#include <GL/glew.h>
#include <glm/glm.hpp>

class ShaderUniform
{
public:
	explicit ShaderUniform(unsigned value) : mLocation(value) {}

	void setFloat(GLfloat value) const noexcept;
	void setFloat1fv(const GLfloat *floats, size_t size) const noexcept;

	void setInteger(GLint value) const noexcept;
	void setInteger1iv(const GLint *ints, size_t size) const noexcept;

	void setVector2f(GLfloat x, GLfloat y) const noexcept;
	void setVector2f(const glm::vec2 &value) const noexcept;
	void setVector2fv(const GLfloat floats[][2], size_t size) const noexcept;

	void setVector3f(GLfloat x, GLfloat y, GLfloat z) const noexcept;
	void setVector3f(const glm::vec3 &value) const noexcept;
	void setVector3fv(const GLfloat floats[][3], size_t size) const noexcept;

	void setVector4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) const noexcept;
	void setVector4f(const glm::vec4 &value) const noexcept;
	void setVector4fv(const GLfloat floats[][4], size_t size) const noexcept;

	void setMatrix4(const glm::mat4 &value) const noexcept;

private:
	unsigned mLocation;
};

class ShaderAttrib
{
public:
	explicit ShaderAttrib(GLint value) : glHandle(value) {}

	GLint getHandle() const { return glHandle; }

private:
	GLint glHandle;
};

class Shader
{
public:
	enum class Type
	{
		VERTEX,
		FRAGMENT,
		GEOMETRY,
		COMPUTE,
	};

	bool loadFromFile(const std::filesystem::path &vs,
	                  const std::filesystem::path &fs) noexcept;

	bool create();
	void destroy();

	void use() const noexcept;
	bool attachFile(Shader::Type type, const std::filesystem::path &path) const;
	bool attachString(Shader::Type type, const std::string& source) const noexcept;
	bool link() const noexcept;

	ShaderUniform getUniform(const std::string& name) const;
	ShaderAttrib getAttrib(const std::string& name) const;

private:
	unsigned mProgram = 0;
};
