#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"

#include <iostream>

// is called each time window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// is called when certain input action observed
void processInput(GLFWwindow* window);

const char* vertexShaderSource = 
	"#version 330 core\n" 
	"layout(location=0) in vec3 aPos;\n"
	"void main()\n"
	"{\n"
	"gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"}\0";

const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

int main()
{
	// intialize GLFW
	glfwInit();
	// specify major OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// specify minor OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// specify OpenGL core-profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// WINDOW CREATION + CHECK
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	} // end if

	// tell GLFW to make the context of our window the main context on the current thread
	glfwMakeContextCurrent(window);

	// INITIALIZE GLAD + CHECK
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to intialize GLAD" << std::endl;
		return -1;
	} // end if

	// tell OpenGL the size of the rendering window
	glViewport(0, 0, 800, 600);

	// tell GLFW to call function on every window resize
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// shader creation
	Shader myShader("Shaders/5 - Hello Triangle/HelloTriangle_vs.glsl", "Shaders/5 - Hello Triangle/HelloTriangle_fs.glsl");

	// triangle vertices
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,		// 0
		-0.5f, 0.5f, 0.0f,		// 1
		0.5f, -0.5f, 0.0f,		// 2
		0.5f, 0.5f, 0.0f		// 3
	};

	// triangle indices
	unsigned int indices[] =
	{
		0, 1, 2,	// first triangle vertices
		1, 2, 3		// second triangle vertices
	};			

	// vertex array object (stores VBO and EBO)
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	// vertex buffer object (stores vertices)
	unsigned int VBO;
	glGenBuffers(1, &VBO);

	// element buffer object (stores indices)
	unsigned int EBO;
	glGenBuffers(1, &EBO);

	// bind vertex array object
	glBindVertexArray(VAO);

	// bind vertex buffer object
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// copy previously defined vertex data into buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// bind element buffer object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// copy previously defined indices data into buffer's memory
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		
	// set the vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// set color to display after clear (state-setting function)
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		// clear the screen colors (state-using function)
		glClear(GL_COLOR_BUFFER_BIT);

		// process any inputs
		processInput(window);


		// use my shader program
		myShader.use();

		// bind/activate the VAO data we stored earlier
		glBindVertexArray(VAO);

		// draw triangles with the VAO set up
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		

		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	} // end while



	// clean/delete all GLFW's resources that were allocated
	glfwTerminate();
	return 0;
} // end main


// this function is called whenever window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

} // end of framebuffer_size_callback(...)


// this function is called whenever an input is observed
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

} // end of processInput(...)

