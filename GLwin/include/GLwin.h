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
    
    // --- new: return native HWND for platform-specific presentation / interop ---
    // Returns NULL if the implementation cannot provide a native handle.
    HWND GLwinGetHWND(GLWIN_window* window);
	// ------------------------------------------  End HWND ------------------------------------------

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
    // --- Backbuffer / DIB helper for zero-copy software rendering ---
    // Create a DIB-section sized to the requested width/height and return a pointer to the pixel bits.
    // The returned pointer is valid until GLwinDestroyBackbuffer is called or the backbuffer is recreated on resize.
    // If width==0 or height==0 the current window framebuffer size is used.
    // Pixel format: 32bpp BGRA (DWORD aligned).
    void* GLwinCreateBackbuffer(GLWIN_window* window, int width, int height, int* outWidth, int* outHeight);
    // Get the pointer to the active backbuffer pixels (or NULL). Useful for rendering directly.
    void* GLwinGetBackbufferPixels(GLWIN_window* window);
    // Destroy the backbuffer created with GLwinCreateBackbuffer.
    void GLwinDestroyBackbuffer(GLWIN_window* window);
    // Present the backbuffer to the window (blit). This is separate from GLwinSwapBuffers so an app can control presentation.
    void GLwinPresentBackbuffer(GLWIN_window* window);



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

    typedef void(*GLwinMouseButtonCallback)(int button, int action, int mods);
    typedef void(*GLwinCursorPosCallback)(double xpos, double ypos);
    typedef void(*GLwinScrollCallback)(double xoffset, double yoffset);

    void GLwinSetMouseButtonCallback(GLWIN_window* window, GLwinMouseButtonCallback cb);
    void GLwinSetCursorPosCallback(GLWIN_window* window, GLwinCursorPosCallback cb);
    void GLwinSetScrollCallback(GLWIN_window* window, GLwinScrollCallback cb);

    // Mouse cursor helpers
    void GLwinSetCursorVisible(GLWIN_window* window, int visible);
    void GLwinSetCursorPos(GLWIN_window* window, int x, int y);

    // File drop (drag & drop) callback: receives count and array of wide-char paths
    typedef void(*GLwinDropCallback)(int count, const wchar_t** paths);
    void GLwinSetDropCallback(GLWIN_window* window, GLwinDropCallback cb);

    // Clipboard helpers
    void GLwinSetClipboardString(GLWIN_window* window, const char* str);
    const char* GLwinGetClipboardString(GLWIN_window* window); // returns internal pointer; copy if you need it

    // Swap interval / vsync control (for apps using GLwinSwapBuffers or software present)
    // Returns previous interval (or 0/-1 on failure).
    int GLwinSetSwapInterval(int interval);

    // Get monitor refresh rate (Hz). Useful for timing/vsync decisions.
    int GLwinGetRefreshRate(GLWIN_window* window);

    // User pointer to attach app-specific data to a window (like GLFW)
    void GLwinSetUserPointer(GLWIN_window* window, void* ptr);
    void* GLwinGetUserPointer(GLWIN_window* window);

    // Window title helper
    void GLwinSetWindowTitle(GLWIN_window* window, const wchar_t* title);


    // Terminate and cleanup library (optional, for symmetry with GLFW)
    void GLwinTerminate(void);


#ifdef __cplusplus
}
#endif 
