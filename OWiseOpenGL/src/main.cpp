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

	// tell GLFW to call function on every window resize
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// INITIALIZE GLAD + CHECK
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to intialize GLAD" << std::endl;
		return -1;
	} // end if

	// enable depth test
	glEnable(GL_DEPTH_TEST);

	// shader creation
	Shader myShader("shaders/9/vs.glsl", "shaders/9/fs.glsl");

	// cube vertices
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};	

	// define cube positions in world
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(2.0f, 5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f, 3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f, 2.0f, -2.5f),
		glm::vec3(1.5f, 0.2f, -1.5f),
		glm::vec3(-1.3f, 1.0f, -1.5f)
	};

	// vertex array object (stores VBO + EBO)
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	// vertex buffer object (stores vertices)
	unsigned int VBO;
	glGenBuffers(1, &VBO);

	// bind vertex array object
	glBindVertexArray(VAO);

	// bind vertex buffer object
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// copy previously defined vertex data into buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		
	// set position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// set texture coordinate attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

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
		// process any inputs
		processInput(window);

		// RENDER //
		// set color to display after clear (state-setting function)
		glClearColor(0.1f, 0.3f, 0.3f, 1.0f);
		// clear the screen colors (state-using function)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind/activate the textures we stored earlier
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);

		// use my shader program
		myShader.use();

		// view transform
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

		// projection transform
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		// set transform uniforms
		myShader.setMat4("view", view);
		myShader.setMat4("projection", projection);

		// bind/activate the VAO data we stored earlier (cube)
		glBindVertexArray(VAO);

		// set transforms for each cube
		for (unsigned int i = 0; i < sizeof(cubePositions) / sizeof(cubePositions[0]); i++)
		{
			// model transform
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * (i + 1.0f);
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			myShader.setMat4("model", model);

			// draw cube
			glDrawArrays(GL_TRIANGLES, 0, 36);
		} // end for
		

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

