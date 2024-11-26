#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../header/shader.hpp"
#include "../header/stb_image.h"

#include <iostream>

// is called each time window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// is called when certain input action observed
void processInput(GLFWwindow* window);

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
	Shader myShader("shaders/8/vs.glsl", "shaders/8/fs.glsl");

	// triangle vertices
	float vertices[] = {
		// positions		// colors		  // texture coords
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
	};

	// triangle indices
	unsigned int indices[] =
	{
		0, 1, 3,	// first triangle vertices
		1, 2, 3		// second triangle vertices
	};		

	// vertex array object (stores VBO + EBO)
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
		
	// set position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// set color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// set texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// texture object
	unsigned int texture;
	glGenTextures(1, &texture);

	// bind texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// set texture wrapping/filtering options (on currently bound texture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("textures/cosmos.jpg", &width, &height, &nrChannels, 0);

	// load successful
	if (data) {
		// generate texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	// bad load
	else {
		std::cout << "Failed to load texture!" << std::endl;
	}

	// free image memory
	stbi_image_free(data);


	// texture1 object
	unsigned int texture1;
	glGenTextures(1, &texture1);

	// bind texture
	glBindTexture(GL_TEXTURE_2D, texture1);

	// set texture wrapping/filtering options (on currently bound texture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load texture1
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("textures/D2_Farewell_End.png", &width, &height, &nrChannels, 0);

	// load successful
	if (data) {
		// generate texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	// bad load
	else {
		std::cout << "Failed to load texture!" << std::endl;
	}

	// free image memory
	stbi_image_free(data);
	
	// set texture unit for shader
	myShader.use();
	myShader.setInt("texture0", 0);
	myShader.setInt("texture1", 1);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// set color to display after clear (state-setting function)
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		// clear the screen colors (state-using function)
		glClear(GL_COLOR_BUFFER_BIT);

		// process any inputs
		processInput(window);


		// bind/activate the textures we stored earlier
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);

		// use my shader program
		myShader.use();

		// rotate and translate matrix transform
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.5f));
		trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0, 0.0, 1.0));

		// set shader transform uniform
		myShader.setMat4("transform", trans);
			
		// set ourColor uniform
		myShader.setVec4("ourColor", glm::vec4(sin(glfwGetTime() / 2) + 0.5, 0.1f, glfwGetTime() * 2, 1.0f));

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

