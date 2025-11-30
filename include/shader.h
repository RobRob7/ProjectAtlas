#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>

class Shader
{
public:
	Shader() = default;
	// constructor
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	// destructor
	~Shader();

	// disallow copy
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	// move constructor
	Shader(Shader&& other) noexcept;
	// move assignment operator
	Shader& operator=(Shader&& other) noexcept;

	void use() const;
	// set various uniforms in shader
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec2(const std::string& name, const glm::vec2& value) const;
	void setVec2(const std::string& name, float x, float y) const;
	void setVec3(const std::string& name, const glm::vec3& value) const;
	void setVec3(const std::string& name, float x, float y, float z) const;
	void setVec4(const std::string& name, const glm::vec4& value) const;
	void setVec4(const std::string& name, float x, float y, float z, float w) const;
	void setMat2(const std::string& name, const glm::mat2& mat) const;
	void setMat3(const std::string& name, const glm::mat3& mat) const;
	void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
	// shader ID
	uint32_t ID_{};
private: 
	// this function check for shader compilation/linking errors
	void checkCompileErrors(GLuint shader, std::string type, std::string path);
};

#endif