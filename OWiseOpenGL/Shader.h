#ifndef SHADER_H
#define SHADER_H


#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class Shader
{
public:
	unsigned int ID;

	// constructor
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
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
			// open shader files
			problemFile = "VERTEX";
			vertexShaderFile.open(vertexPath);
			problemFile = "FRAGMENT";
			fragmentShaderFile.open(fragmentPath);

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

			// check for geometry shader
			if (geometryPath != nullptr)
			{
				problemFile = "GEOMETRY";
				geometryShaderFile.open(geometryPath);
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

		// will contain ID's for shaders
		unsigned int vertex, fragment;

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
		unsigned int geometry;
		if (geometryPath != nullptr)
		{
			const char* geometryShaderCode = geometryCode.c_str();
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &geometryShaderCode, NULL);
			glCompileShader(geometry);
			checkCompileErrors(geometry, "GEOMETRY", geometryPath);
		}

		// create shader program
		ID = glCreateProgram();

		// attach shaders to program
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		if (geometryPath != nullptr)
			glAttachShader(ID, geometry);
	
		// link program
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM", "");

		// delete the shaders (no longer needed)
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (geometryPath != nullptr)
			glDeleteShader(geometry);

	} // end of constructor


	// this function activates the current shader
	void use() const
	{
		glUseProgram(ID);

	} // end of use()

private:

	// this function check for shader compilation/linking errors
	void checkCompileErrors(GLuint shader, std::string type, std::string path)
	{
		GLint success;
		GLchar infoLog[1024];

		if (!std::strcmp(type.c_str(), "PROGRAM"))
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


	


};

#endif