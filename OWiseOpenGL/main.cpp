#include <glad/glad.h>
#include <GLFW/glfw3.h>

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



	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// set color to display after clear (state-setting function)
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		// clear the screen colors (state-using function)
		glClear(GL_COLOR_BUFFER_BIT);

		// process any inputs
		processInput(window);
		

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

