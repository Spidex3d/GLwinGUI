#pragma once
#include <windows.h>
#include <string>
#include "GLwinDefs.h"


#ifdef __cplusplus
extern "C" {
#endif

	void GLwinHelloFromGLwin();

    // Opaque window structure
    typedef struct GLWIN_window GLWIN_window;


    // Window creation & destruction
    GLWIN_window* GLwin_CreateWindow(int width, int height, const wchar_t* title);
    void GLwin_DestroyWindow(GLWIN_window* window);

    // Window/context management
    void GLwinMakeContextCurrent(GLWIN_window* window);
    void GLwinSwapBuffers(GLWIN_window* window);
    void GLwinPollEvents(void);
    int  GLwinWindowShouldClose(GLWIN_window* window);
    // Window hints (Maximize, Resizeabel, Done)
   //TO DO GLWIN_CONTEXT_VERSION_MAJOR, GLWIN_CONTEXT_VERSION_MINOR, GLWIN_OPENGL_PROFILE, GLWIN_OPENGL_CORE_PROFILE
    void GLwinWindowHint(int hint, int value);
    // Set window position
    // set window opacity
    // set window focus
    // get window refresh rate

    // Framebuffer size and window state
    void GLwinGetFramebufferSize(GLWIN_window* window, int* width, int* height);
    int  GLwinGetWidth(GLWIN_window* window);
    int  GLwinGetHeight(GLWIN_window* window);
    // Optional: callback for resize
    typedef void(*GLwinResizeCallback)(int width, int height);

    // Window icon and maximize
    void GLwinSetWindowIcon(GLWIN_window* window, const wchar_t* iconPath);

    // Time API
    // getTime
    // getElapsedTime
    // getFrameTime
    // setTargetFPS
    // waitTime



    // Keyboard input API
    int GLwinGetKey(GLWIN_window* window, int keycode);

    // Mouse input API
    int GLwinGetMouseButton(GLWIN_window* window, int button);
    void GLwinGetCursorPos(GLWIN_window* window, double* xpos, double* ypos);

    // GLwinUI: Simple GUI message box
    // GLwinUI_MessageBox(const wchar_t* title, const wchar_t* message);

    // Font loading API
    // Text drawing on screen like raylib

    // Basic 2D drawing API
    // Sprite drawing 

    // object loading API

    // Shader loading API

    // Sound loading API

    // Terminate and cleanup library (optional, for symmetry with GLFW)
    void GLwinTerminate(void);


#ifdef __cplusplus
}
#endif 
