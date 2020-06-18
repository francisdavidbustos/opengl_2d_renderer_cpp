#include <GLFW/glfw3.h>

int main() {

	if (!glfwInit()) {
		// Initialization failed
	}

	GLFWwindow* window = glfwCreateWindow(480, 360, "Renderer", NULL, NULL);
	if (!window) {
		// Window or OpenGL context creation failed
	}

	glfwMakeContextCurrent(window);

	int width = 480, height = 360;
	while (!glfwWindowShouldClose(window)) {
		// Keep running

		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, 480, 360);
		glfwSwapBuffers(window);
		glfwSwapInterval(1);
	}
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}