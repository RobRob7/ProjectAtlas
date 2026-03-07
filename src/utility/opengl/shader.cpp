#include "shader.h"

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

//--- PUBLIC ---//
Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vertexShaderFile;
	std::ifstream fragmentShaderFile;
	std::ifstream geometryShaderFile;

	// will contain the problem file trying to open
	std::string problemFile{};

	// ensure ifstream objects can throw exceptions
	vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	geometryShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// preliminary path to shaders
		const std::string pathToShaders = std::string(RESOURCES_PATH) + "/shader/";
		// open shader files
		problemFile = "VERTEX";
		vertexShaderFile.open(pathToShaders + vertexPath);
		problemFile = "FRAGMENT";
		fragmentShaderFile.open(pathToShaders + fragmentPath);

		// will contain file info as stringstrean for each shader
		std::stringstream vertexShaderStream;
		std::stringstream fragmentShaderStream;

		// read file's buffer contents into streams
		vertexShaderStream << vertexShaderFile.rdbuf();
		fragmentShaderStream << fragmentShaderFile.rdbuf();

		// close file handlers
		vertexShaderFile.close();
		fragmentShaderFile.close();

		// convert stringstream to string
		vertexCode = vertexShaderStream.str();
		fragmentCode = fragmentShaderStream.str();

		//#ifdef _DEBUG
		//std::cout << vertexCode << "\n\n" << fragmentCode << "\n";
		//#endif

		// check for geometry shader
		if (geometryPath != nullptr)
		{
			problemFile = "GEOMETRY";
			geometryShaderFile.open(pathToShaders + geometryPath);
			std::stringstream geometryShaderStream;
			geometryShaderStream << geometryShaderFile.rdbuf();
			geometryShaderFile.close();
			geometryCode = geometryShaderStream.str();
		}
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::" << problemFile << "_SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
	}

	// contains the shader code
	const char* vertexShaderCode = vertexCode.c_str();
	const char* fragmentShaderCode = fragmentCode.c_str();

	// will contain ID_'s for shaders
	uint32_t vertex, fragment;

	// vertex shader compile
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX", vertexPath);

	// fragment shader compile
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT", fragmentPath);

	// check for geometry shader and compile if needed
	uint32_t geometry;
	if (geometryPath != nullptr)
	{
		const char* geometryShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &geometryShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY", geometryPath);
	}

	// create shader program
	ID_ = glCreateProgram();

	// attach shaders to program
	glAttachShader(ID_, vertex);
	glAttachShader(ID_, fragment);
	if (geometryPath != nullptr)
		glAttachShader(ID_, geometry);
	
	// link program
	glLinkProgram(ID_);
	checkCompileErrors(ID_, "PROGRAM", "");

	// delete the shaders (no longer needed)
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr)
		glDeleteShader(geometry);

	uniformLocationCache_.clear();
} // end of constructor

Shader::~Shader()
{
	if (ID_ != 0)
	{
		glDeleteProgram(ID_);
		ID_ = 0;
	}
} // end of destructor

Shader::Shader(Shader&& other) noexcept
: ID_(other.ID_), uniformLocationCache_(std::move(other.uniformLocationCache_))
{
	other.ID_ = 0;
} // end of move constructor

Shader& Shader::operator=(Shader&& other) noexcept
{
	if (this != &other)
	{
		if (ID_ != 0) glDeleteProgram(ID_);

		ID_ = other.ID_;
		uniformLocationCache_ = std::move(other.uniformLocationCache_);

		other.ID_ = 0;
	}
	return *this;
} // end of move assignment

void Shader::use() const
{
	glUseProgram(ID_);
} // end of use()

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(getUniformLocation(name), (int)value);
} // end of setBool()

void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(getUniformLocation(name), value);
} // end of setInt()

void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(getUniformLocation(name), value);
} // end of setFloat()

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(getUniformLocation(name), 1, &value[0]);
} // end of setVec2()

void Shader::setVec2(const std::string& name, float x, float y) const
{
	glUniform2f(getUniformLocation(name), x, y);
} // end of setVec2()

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(getUniformLocation(name), 1, &value[0]);
} // end of setvec3

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(getUniformLocation(name), x, y, z);
} // end of setVec3s()

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(getUniformLocation(name), 1, &value[0]);
} // end of setVec4()

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
	glUniform4f(getUniformLocation(name), x, y, z, w);
} // end of setVec4()

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
	glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
} // end of setMat2()

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
} // end of setMat3()

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
} // end of setMat4()

//--- PRIVATE ---//
int32_t Shader::getUniformLocation(std::string_view name) const
{
	std::string key(name);

	auto it = uniformLocationCache_.find(key);
	if (it != uniformLocationCache_.end())
	{
		return it->second;
	}

	int32_t location = glGetUniformLocation(ID_, key.c_str());

#ifdef _DEBUG
	if (location == -1) {
		std::cout << "WARNING::UNIFORM '" << name << "' not found in shader!\n";
	}
#endif
	
	uniformLocationCache_.emplace(std::move(key), location);
	return location;
} // end of getUniformLocation()

void Shader::checkCompileErrors(uint32_t shader, std::string_view type, std::string_view path)
{
	GLint success;
	GLchar infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "-" << path << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
} // end of checkCompileErrors(...)