#pragma once

#include <filesystem>
#include <glm/glm.hpp>

class ShaderUniform
{
public:
	explicit ShaderUniform(unsigned value) : mLocation(value) {}

	void setFloat(float value) const noexcept;
	void setFloat1fv(const float *floats, size_t size) const noexcept;

	void setInteger(int value) const noexcept;
	void setInteger1iv(const int *ints, size_t size) const noexcept;

	void setVector2f(float x, float y) const noexcept;
	void setVector2f(const glm::vec2 &value) const noexcept;
	void setVector2fv(const float floats[][2], size_t size) const noexcept;

	void setVector3f(float x, float y, float z) const noexcept;
	void setVector3f(const glm::vec3 &value) const noexcept;
	void setVector3fv(const float floats[][3], size_t size) const noexcept;

	void setVector4f(float x, float y, float z, float w) const noexcept;
	void setVector4f(const glm::vec4 &value) const noexcept;
	void setVector4fv(const float floats[][4], size_t size) const noexcept;

	void setMatrix4(const glm::mat4 &value) const noexcept;

private:
	unsigned mLocation;
};

class ShaderAttrib
{
public:
	explicit ShaderAttrib(int value) : mLocation(value) {}

	int getHandle() const { return mLocation; }

private:
	unsigned mLocation;
};

class Shader
{
public:
	enum class Type
	{
		Vertex,
		Fragment,
		Geometry,
		Compute,
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
