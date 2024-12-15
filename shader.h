#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>

class ShaderUniform
{
public:
	explicit ShaderUniform(GLint value) : glHandle(value) {}

	void SetFloat(GLfloat value) const noexcept;
	void SetFloat1fv(const GLfloat *floats, size_t size) const noexcept;

	void SetInteger(GLint value) const noexcept;
	void SetInteger1iv(const GLint *ints, size_t size) const noexcept;

	void SetVector2f(GLfloat x, GLfloat y) const noexcept;
	void SetVector2f(const glm::vec2 &value) const noexcept;
	void SetVector2fv(const GLfloat floats[][2], size_t size) const noexcept;

	void SetVector3f(GLfloat x, GLfloat y, GLfloat z) const noexcept;
	void SetVector3f(const glm::vec3 &value) const noexcept;
	void SetVector3fv(const GLfloat floats[][3], size_t size) const noexcept;

	void SetVector4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) const noexcept;
	void SetVector4f(const glm::vec4 &value) const noexcept;
	void SetVector4fv(const GLfloat floats[][4], size_t size) const noexcept;

	void SetMatrix4(const glm::mat4 &value) const noexcept;

private:
	GLint glHandle;
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

	GLuint ID;

	int  Create();
	void Destroy();

	void Use() const;
	int  Attach(Shader::Type type, const char *source) const;
	int  Link() const;

	ShaderUniform GetUniform(const char *name) const;
	ShaderAttrib GetAttrib(const char *name) const;

private:
	static int checkShaderCompilation(GLuint object);
	static int checkProgramLinkage(GLuint object);
};

#endif
