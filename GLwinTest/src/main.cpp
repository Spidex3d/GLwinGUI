#include <glad/glad.h>// Include glad to get the OpenGL headers best if glad is at the top
#include "../../GLwinGUI/vendors/glm/glm.hpp" // Ensure this include is correct and resolves glm::mat4  
#include <iostream>
#include <locale>
#include <GLwin.h>  // Include the GLwin header file my GLFW
#include <GLwinLog.h> // Include the GLwin logging header

// The new code is on GitHub : https://github.com/Spidex3d/GLwin

// It's a window library API to make it easy to open a window and set up modern opengl like GLFW _ SLD_Raylib
// Road map get a working window open and then add more features, 
// ############################ Always run before adding new code ############################

// This is your callback function
void MyCharCallback(unsigned int codepoint) {
	// Print the Unicode codepoint as a character (if printable)
	printf("Char input: U+%04X '%lc'\n", codepoint, (wchar_t)codepoint);
}
void MyKeyCallback(int key, int action) {
	// Print key event details
	const char* act = (action == GLWIN_PRESS) ? "pressed" : "released";
	printf("Key: %d %s\n", key, act);
}
// Convert a wide (UTF-16) C-string to UTF-8 std::string using Win32 API
static std::string WideToUtf8(const wchar_t* wstr) {
	if (!wstr) return std::string();
	int required = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	if (required <= 0) return std::string();
	std::string out;
	out.resize(required - 1); // exclude null terminator
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &out[0], required, nullptr, nullptr);
	return out;
}

// Drop callback signature:
// typedef void(*GLwinDropCallback)(int count, const wchar_t** paths);
void OnDrop(int count, const wchar_t** paths) {
	if (count <= 0 || !paths) {
		std::wcout << L"[Drop] empty\n";
		return;
	}

	std::wcout << L"[Drop] Received " << count << L" path(s):\n";
	for (int i = 0; i < count; ++i) {
		std::wcout << L"  " << paths[i] << L"\n";
	}

	// Convert to UTF-8 using WideCharToMultiByte and print on std::cout
	for (int i = 0; i < count; ++i) {
		std::string utf8 = WideToUtf8(paths[i]);
		std::cout << "[Drop utf8]  " << utf8 << std::endl;
	}
}


int main() {
	// Optional: enable wide output on Windows console (so std::wcout shows nice paths)
	// Note: This is a convenience; your environment may already handle wide output.
	std::locale::global(std::locale(""));

	std::cout << "Hello, GLwinTest main Window!" << std::endl;
	GLwinHelloFromGLwin();
	

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
	
	GLwinSetDropCallback(window, OnDrop);

	//GLwinSetCursorVisible(window, GLWIN_FALSE); // Show cursor

	// Toggle custom title bar on/off
	//GLwinEnableCustomTitleBar(window, GLWIN_TRUE);  // Enable custom title bar - ON
	GLwinEnableCustomTitleBar(window, GLWIN_FALSE); // Restore Windows default - OFF

	// Set a custom icon (make sure "icon.ico" exists in working directory next to the .exe file)
	GLwinSetWindowIcon(window, L"icon_01.ico");
	GLwinMakeContextCurrent(window);
	int w, h;
	GLwinGetFramebufferSize(window, &w, &h);
	GLWIN_LOG_DEBUG("Framebuffer x= " << w << ", y= " << h);

	

	int winX, winY;
	GLwinGetWindowPos(window, &winX, &winY);  // fills winX & winY
	std::cout << "Window Pos: " << winX << ", " << winY << std::endl;

	GLwinGetGlobalCursorPos(window, &winX, &winY);
	std::cout << "Cursor Pos: " << winX << ", " << winY << std::endl;

	int newX = 200, newY = 100;
	GLwinSetWindowPos(window, newX, newY);

	GLwinSetCharCallback(window, MyCharCallback);
	GLwinSetKeyCallback(window, MyKeyCallback);

	GLwinCharCallback charCallback = [](unsigned int codepoint) {
		std::cout << "Char input: " << static_cast<char>(codepoint) << " (codepoint: " << codepoint << ")" << std::endl;
	};

	if (gladLoadGLLoader((GLADloadproc)GLwinGetProcAddress) == 0) {
		GLWIN_LOG_WARNING("Failed to initialize GLAD with GLwinGetProcAddress!");
		return -1;
	}
	else {
		GLWIN_LOG_INFO("GLAD initialized successfully with GLwinGetProcAddress.");
	}
		
	
	glGetString(GL_VERSION); // Ensure context is current
	GLWIN_LOG_INFO("OpenGL version " << glGetString(GL_VERSION));

	const double targetFPS = 60.0; // or 120.0
	const double targetFrameTime = 1.0 / targetFPS; // in seconds

	GLwinSetWindowTitle(window, L"Changed Window Title");

	while (!GLwinWindowShouldClose(window, 0)) {
		double frameStart = GLwinGetTime(); // Start time of the frame

		GLwinPresentBackbuffer(window);			 // blit to window
		// Poll and handle events (inputs, window resize, etc.)
		GLwinPollEvents(); // New non-blocking event polling

		if (GLwinGetKey(window, GLWIN_ESCAPE) == GLWIN_PRESS) {
			std::cout << "Escape key pressed, closing window." << std::endl;
			//break; // Exit loop to close window
			GLwinWindowShouldClose(window, 1);
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
		
		// -------------------------------- Rendering code goes here --------------------------------

		GLwinSwapBuffers(window);

		// Throttle to target FPS
		double frameEnd = GLwinGetTime();
		double elapsed = frameEnd - frameStart;
		if (elapsed < targetFrameTime) {
			double waitTime = targetFrameTime - elapsed;
			DWORD ms = (DWORD)(waitTime * 1000.0);
			if (ms > 0) Sleep(ms); // Sleep for the remaining time
			// Spin-wait for extra precision (optional)
			while ((GLwinGetTime() - frameStart) < targetFrameTime) {}
		}

		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
			GLWIN_LOG_ERROR("Something went wrong with opengl: " << err);

	}

	GLwin_DestroyWindow(window); // Clean up and close window
	GLwinTerminate();

	GLWIN_LOG_INFO("Window closed, exiting.");

	return 0;
}