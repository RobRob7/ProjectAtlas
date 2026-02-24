#include "opengl_main.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

//--- PUBLIC ---//
OpenGLMain::OpenGLMain() = default;

OpenGLMain::~OpenGLMain() = default;

void OpenGLMain::init()
{
	// INITIALIZE GLAD + CHECK
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("GLAD initialization failure!");
	} // end if
} // end of init()