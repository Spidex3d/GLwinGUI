#include <glad/glad.h>// Include glad to get the OpenGL headers
#include <iostream>
#include <GLwin.h>  // Include the GLwin header file my GLFW
#include <GLwinGUI.h>
#include <GLwinLog.h> // Include the GLwin logging header
 

// The new code is on GitHub : https://github.com/Spidex3d/GLwin

// It's a window library API to make it easy to open a window and set up modern opengl like GLFW _ SLD_Raylib
// Road map get the window open, have a GUI, have minimal 2d and 3d objects, have a minimal Shader, have a.obj model loader

// ############################ Always run before adding new code ############################

int main() {
	std::cout << "Hello, GLwinTest main Window!" << std::endl;
	GLwinHelloFromGLwin();

	GLwinGUI::Instance()->GLwinHelloFromGLwinGUI();
	

	GLwinWindowHint(0, 0); // Not implemented
	GLwinWindowHint(GLWIN_MAXIMIZED, GLWIN_FALSE); // Default is not maximized
	GLwinWindowHint(GLWIN_RESIZABLE, GLWIN_TRUE); // Default is resizable

	GLWIN_window* window = GLwin_CreateWindow(1200, 600, L"Starting GLwin! with Modern OpenGL");

	if (!window) {
		GLWIN_LOG_WARNING("Failed to create window!");
		return 1;
	}
	else {
		GLWIN_LOG_INFO("Window created successfully.");
	}

	// Set a custom icon (make sure "icon.ico" exists in working directory next to the .exe file)
	GLwinSetWindowIcon(window, L"icon_01.ico");
	GLwinMakeContextCurrent(window);
	int w, h;
	GLwinGetFramebufferSize(window, &w, &h);
	//std::cout << "Framebuffer size: " << w << " x " << h << std::endl;
	GLWIN_LOG_DEBUG("Framebuffer x= " << w << ", y= " << h);

	/*if (!gladLoadGL()) {

		GLWIN_LOG_WARNING("Failed to initialize GLAD!");
		return 1;
	}
	else {
		GLWIN_LOG_INFO("GLAD initialized successfully.");
	}*/
	 GLwinGUI::Instance()->Initialize(); // Initialize GLAD after context creation
	//GLwinGUI::Instance()->CreateGuiWindow("Test Window", 50, 50, 300, 200);
	
	


	glGetString(GL_VERSION); // Ensure context is current
	GLWIN_LOG_INFO("OpenGL version " << glGetString(GL_VERSION));

	while (!GLwinWindowShouldClose(window)) {
		// Poll and handle events (inputs, window resize, etc.)
		GLwinPollEvents(); // New non-blocking event polling

		if (GLwinGetKey(window, GLWIN_ESCAPE) == GLWIN_PRESS) {
			std::cout << "Escape key pressed, closing window." << std::endl;
			break; // Exit loop to close window
		}
		if (GLwinGetKey(window, GLWIN_SPACE) == GLWIN_PRESS) {
			std::cout << "Space key is being held down." << std::endl;

		}
		// mouse input
		double mx, my;
		GLwinGetCursorPos(window, &mx, &my);
		if (GLwinGetMouseButton(window, GLWIN_MOUSE_BUTTON_LEFT) == GLWIN_PRESS) {
			printf("Mouse Left Button Pressed at (%f, %f)\n", mx, my);
		}


		

		glClearColor(0.17f, 0.17f, 0.18f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, w, h);

		GLwinGUI::Instance()->GLwinDrawGuiWindow(); // Draw GUI windows
		


		GLwinSwapBuffers(window);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
			GLWIN_LOG_ERROR("Something went wrong with opengl: " << err);

	}
	GLwin_DestroyWindow(window); // Clean up and close window
	GLwinTerminate();

	std::wcout << L"Window closed, exiting.\n";

	return 0;
}