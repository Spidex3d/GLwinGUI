#pragma once
#include <windows.h>
#include <string>
#include "GLwinDefs.h"
#include "GLwinTime.h"
#include "GLwinDialog.h"


#ifdef __cplusplus
extern "C" {
#endif

	void GLwinHelloFromGLwin();

    // Opaque window structure
    typedef struct GLWIN_window GLWIN_window;
    

    // Window creation & destruction
    GLWIN_window* GLwin_CreateWindow(int width, int height, const wchar_t* title);
    void GLwin_DestroyWindow(GLWIN_window* window);
	// Enable or disable custom title bar
    void GLwinEnableCustomTitleBar(GLWIN_window* window, int enable);

    // Window/context management
    void GLwinMakeContextCurrent(GLWIN_window* window);
	void* GLwinGetProcAddress(const char* procname);
    void GLwinSwapBuffers(GLWIN_window* window);
    void GLwinPollEvents(void);
    //int  GLwinWindowShouldClose(GLWIN_window* window);
    bool GLwinWindowShouldClose(GLWIN_window* window, bool close);
	void GLwinRestoreWindow(GLWIN_window* window);
	void GLwinMinimizeWindow(GLWIN_window* window);
	void GLwinMaximizeWindow(GLWIN_window* window);

    // Window hints (Maximize, Resizeabel, Done)
   //TO DO GLWIN_CONTEXT_VERSION_MAJOR, GLWIN_CONTEXT_VERSION_MINOR, GLWIN_OPENGL_PROFILE, GLWIN_OPENGL_CORE_PROFILE
    void GLwinWindowHint(int hint, int value);
    
    // set window opacity
    // set window focus
    // get window refresh rate

    // Framebuffer size and window state
    void GLwinGetFramebufferSize(GLWIN_window* window, int* width, int* height);
    void GLwinGetWindowPos(GLWIN_window* window, int* winX, int* winY); // wrapper for current OS window pos
    void GLwinSetWindowPos(GLWIN_window* window, int posX, int posY);
    int  GLwinGetWidth(GLWIN_window* window);
    int  GLwinGetHeight(GLWIN_window* window);
    // Optional: callback for resize
    typedef void(*GLwinResizeCallback)(int width, int height);

    // Window icon and maximize
    void GLwinSetWindowIcon(GLWIN_window* window, const wchar_t* iconPath);

    // Time API
	void GLwinGetTimer(GLWIN_window* window, int tstart, int tmax);
    // getTime
    // getElapsedTime
    // getFrameTime
    // setTargetFPS
    // waitTime



    // Keyboard input API
    int GLwinGetKey(GLWIN_window* window, int keycode);

	typedef void(*GLwinKeyCallback)(int key, int action);
	void GLwinSetKeyCallback(GLWIN_window* window, GLwinKeyCallback callback);
	// Set keyboard callback
    typedef void(*GLwinCharCallback)(unsigned int codepoint);
    void GLwinSetCharCallback(GLWIN_window* window, GLwinCharCallback callback);

    // Mouse input API
    int GLwinGetMouseButton(GLWIN_window* window, int button);
	void GLwinGetGlobalCursorPos(GLWIN_window* window, int* x, int* y);         // screen coordinates
	void GLwinGetClientScreenOrigin(GLWIN_window* window, int* outX, int* outY); // where the window is on the screen
    void GLwinGetCursorPos(GLWIN_window* window, double* xpos, double* ypos);

    // Sound loading API

    // Terminate and cleanup library (optional, for symmetry with GLFW)
    void GLwinTerminate(void);


#ifdef __cplusplus
}
#endif 
