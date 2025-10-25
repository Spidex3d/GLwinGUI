How to set up a window with GLwin

#include <iostream>
#include "../spxgl.h"
#include "GLwin.h" // my version of glfw
#include "GLwinTime.h"
#include "GLwinLog.h"

int main() {
	
	GLWIN_window* window = GLwin_CreateWindow(800, 600, L"SpxGL Window");
	if (!window) {
		GLWIN_LOG_INFO("Window creation failed");
		return -1;
	}
	else {
		GLWIN_LOG_INFO("Window created successfully");
	}

	GLwinMakeContextCurrent(window);

	while (!GLwinWindowShouldClose(window, 0)) {
			
		if (GLwinGetKey(window, GLWIN_ESCAPE) == GLWIN_PRESS) {
			GLwinWindowShouldClose(window, 1);
		}

		// ------ Render here ----------

		// Swap buffers and poll events
		GLwinSwapBuffers(window);
		GLwinPollEvents();
	}
	GLWIN_LOG_INFO("Window is closing, & cleaning up.");
	return 0;
}
