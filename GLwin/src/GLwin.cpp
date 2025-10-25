#include "../GLwin.h"
#include <iostream>

#include "../GLwinLog.h"

#include <windows.h>

#include <iostream>
#include <unordered_map>

// for testing
void GLwinHelloFromGLwin()
{
   
    GLWIN_LOG_INFO("Hello, GLwin.h Window!");
}


// windows hints
static int g_GLwinMaximizedHint = 0;
static int g_GLwinResizableHint = 1; // Default to resizable

// Internal struct definition
struct GLWIN_window {
    HWND hwnd = nullptr;
    HDC hdc = nullptr;
    HGLRC hglrc = nullptr;
    int width = 0, height = 0;
    bool closed = false;
    std::unordered_map<int, bool> keyState;
    std::unordered_map<int, bool> prevKeyState;
    GLwinResizeCallback resizeCallback = nullptr;
    // mouse state
    double mouseX = 0.0, mouseY = 0.0;
    bool mouseButtons[3] = { false, false, false };
	GLwinKeyCallback keyCallback = nullptr;
    GLwinCharCallback charCallback = nullptr;

    // Backbuffer-related fields (for zero-copy CreateDIBSection)
    HBITMAP backBitmap = NULL;    // DIB section HBITMAP
    void* backPixels = nullptr; // pointer to DIB bits (BGRA, top-down)
    int     backWidth = 0;
    int     backHeight = 0;
    HDC     backMemDC = NULL;     // memory DC with the backBitmap selected
    HBITMAP backOldBitmap = NULL; // previous bitmap selected in backMemDC

    // Mouse/cursor/scroll callbacks
    GLwinMouseButtonCallback mouseButtonCallback = nullptr;
    GLwinCursorPosCallback  cursorPosCallback = nullptr;
    GLwinScrollCallback     scrollCallback = nullptr;

	void* userPointer = nullptr; // for user data

	
};

// Internal static
static const wchar_t* GLWIN_WINDOW_CLASS = L"GLWIN_WindowClass";
static bool classRegistered = false;

// Helper: Set pixel format for OpenGL
static bool SetPixelFormatForGL(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(hdc, &pfd);
    if (pf == 0) return false;
    if (!SetPixelFormat(hdc, pf, &pfd)) return false;
    return true;
}

// Forward declaration
static LRESULT CALLBACK GLwin_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// -----------------------------------------------------------------------------
// Backbuffer helpers (CreateDIBSection-backed, zero-copy)
// -----------------------------------------------------------------------------

// Helper: create or recreate the DIB-section backbuffer. Returns true on success.
static int glwin_internal_create_backbuffer(GLWIN_window* window, int reqW, int reqH)
{
    if (!window || !window->hwnd) return 0;

    int w = reqW;
    int h = reqH;
    if (w <= 0 || h <= 0) {
        // use current client size if request is zero/invalid
        RECT rc;
        if (GetClientRect(window->hwnd, &rc)) {
            w = rc.right - rc.left;
            h = rc.bottom - rc.top;
        }
        else {
            w = window->width ? window->width : 1;
            h = window->height ? window->height : 1;
        }
    }

    // If backbuffer exists with same size, keep it
    if (window->backBitmap && window->backWidth == w && window->backHeight == h) {
        return 1;
    }

    // Destroy existing backbuffer if any
    if (window->backMemDC) {
        if (window->backOldBitmap) SelectObject(window->backMemDC, window->backOldBitmap);
        DeleteDC(window->backMemDC);
        window->backMemDC = NULL;
        window->backOldBitmap = NULL;
    }
    if (window->backBitmap) {
        DeleteObject(window->backBitmap);
        window->backBitmap = NULL;
        window->backPixels = NULL;
    }

    // Prepare BITMAPINFO for a top-down 32bpp BGRA DIB (negative height => top-down)
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // 32bpp
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;

    // CreateDIBSection - pass a screen DC for compatibility
    HDC hdcScreen = GetDC(NULL);
    void* bits = NULL;
    HBITMAP hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    ReleaseDC(NULL, hdcScreen);

    if (!hBitmap || !bits) {
        if (hBitmap) DeleteObject(hBitmap);
        return 0;
    }

    // Create a memory DC and select the bitmap
    HDC memDC = CreateCompatibleDC(NULL);
    if (!memDC) {
        DeleteObject(hBitmap);
        return 0;
    }
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hBitmap);
    // It's OK if oldBmp is NULL (when DC was empty).

    // Store into window
    window->backBitmap = hBitmap;
    window->backPixels = bits;
    window->backWidth = w;
    window->backHeight = h;
    window->backMemDC = memDC;
    window->backOldBitmap = oldBmp;

    return 1;
}

#ifdef __cplusplus
extern "C" {
#endif

    // Create a DIB-section sized to the requested width/height and return a pointer to the pixel bits.
    // Pixel format: 32bpp BGRA (top-down). outWidth/outHeight return actual size.
    void* GLwinCreateBackbuffer(GLWIN_window* window, int width, int height, int* outWidth, int* outHeight)
    {
        if (!window) return NULL;

        if (!glwin_internal_create_backbuffer(window, width, height)) {
            if (outWidth) *outWidth = 0;
            if (outHeight) *outHeight = 0;
            return NULL;
        }

        if (outWidth) *outWidth = window->backWidth;
        if (outHeight) *outHeight = window->backHeight;
        return window->backPixels;
    }

    void* GLwinGetBackbufferPixels(GLWIN_window* window)
    {
        if (!window) return NULL;
        return window->backPixels;
    }

    void GLwinDestroyBackbuffer(GLWIN_window* window)
    {
        if (!window) return;

        if (window->backMemDC) {
            if (window->backOldBitmap) {
                SelectObject(window->backMemDC, window->backOldBitmap);
                window->backOldBitmap = NULL;
            }
            DeleteDC(window->backMemDC);
            window->backMemDC = NULL;
        }
        if (window->backBitmap) {
            DeleteObject(window->backBitmap);
            window->backBitmap = NULL;
        }
        window->backPixels = NULL;
        window->backWidth = 0;
        window->backHeight = 0;
    }

    void GLwinPresentBackbuffer(GLWIN_window* window)
    {
        if (!window || !window->hwnd) return;

        if (!window->backBitmap || !window->backMemDC) {
            // Nothing to present
            return;
        }

        // Get window client size to determine destination rectangle
        RECT rc;
        if (!GetClientRect(window->hwnd, &rc)) {
            return;
        }
        int dstW = rc.right - rc.left;
        int dstH = rc.bottom - rc.top;
        if (dstW <= 0 || dstH <= 0) return;

        HDC hdcWindow = GetDC(window->hwnd);
        if (!hdcWindow) return;

        // If backbuffer size matches client size, use BitBlt for 1:1 copy.
        if (window->backWidth == dstW && window->backHeight == dstH) {
            BitBlt(hdcWindow, 0, 0, dstW, dstH, window->backMemDC, 0, 0, SRCCOPY);
        }
        else {
            // Stretch/scale if sizes differ
            SetStretchBltMode(hdcWindow, HALFTONE);
            StretchBlt(hdcWindow, 0, 0, dstW, dstH, window->backMemDC, 0, 0, window->backWidth, window->backHeight, SRCCOPY);
        }

        ReleaseDC(window->hwnd, hdcWindow);
    }

#ifdef __cplusplus
}
#endif
// -----------------------------------------------------------------------------
// Backbuffer helpers (CreateDIBSection-backed, zero-copy)   #### END ####
// -----------------------------------------------------------------------------


//void GLwinHello(void) {
//    std::cout << "Hello, GLwin API Test!" << std::endl;
//    int errorCode = 42; // Example value
//    int x = 1, y = 2;
//    GLWIN_LOG_INIT("GLwin: Logging initialized.");
//
//    GLWIN_LOG_ERROR("Something went wrong: " << errorCode);
//    GLWIN_LOG_WARNING("This might be a problem");
//    GLWIN_LOG_INFO("Starting process X");
//    GLWIN_LOG_TRACE("Tracing process X");
//    GLWIN_LOG_DEBUG("x=" << x << ", y=" << y);
//}
// ------------------------------------------ New code to do with spxgl -------------------------------------
HWND GLwinGetHWND(GLWIN_window* window)
{
    if (!window) return NULL;
    return window->hwnd;
}

GLWIN_window* GLwin_CreateWindow(int width, int height, const wchar_t* title) {
    if (!classRegistered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = GLwin_WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = GLWIN_WINDOW_CLASS;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        if (!RegisterClass(&wc)) return nullptr;
        classRegistered = true;
    }

    GLWIN_window* win = new GLWIN_window();
    win->width = width;
    win->height = height;
    // Window hints
    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!g_GLwinResizableHint) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX); // Remove resizing and maximize
    }

    HWND hwnd = CreateWindowEx(
        0,
        GLWIN_WINDOW_CLASS,
        title,
        style, //
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, GetModuleHandle(nullptr), win);



    if (!hwnd) {
        delete win;
        return nullptr;
    }
    win->hwnd = hwnd;

    // windows hints
    // Apply maximized hint BEFORE showing window
    if (g_GLwinMaximizedHint) {
        ShowWindow(hwnd, SW_MAXIMIZE);
    }
    else {
        ShowWindow(hwnd, SW_SHOW);
    }


    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Setup OpenGL
    win->hdc = GetDC(hwnd);
    if (!win->hdc || !SetPixelFormatForGL(win->hdc)) {
        DestroyWindow(hwnd);
        delete win;
        return nullptr;
    }

    win->hglrc = wglCreateContext(win->hdc);
    if (!win->hglrc) {
        ReleaseDC(hwnd, win->hdc);
        DestroyWindow(hwnd);
        delete win;
        return nullptr;
    }
    if (!wglMakeCurrent(win->hdc, win->hglrc)) {
        wglDeleteContext(win->hglrc);
        ReleaseDC(hwnd, win->hdc);
        DestroyWindow(hwnd);
        delete win;
        return nullptr;
    }

    return win;
}

void GLwin_DestroyWindow(GLWIN_window* window) {
    if (!window) return;

	// Destroy backbuffer if any presant
	GLwinDestroyBackbuffer(window);

    if (window->hglrc) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(window->hglrc);
        window->hglrc = nullptr;
    }
    if (window->hdc && window->hwnd) {
        ReleaseDC(window->hwnd, window->hdc);
        window->hdc = nullptr;
    }
    if (window->hwnd) {
        DestroyWindow(window->hwnd);
        window->hwnd = nullptr;
    }
    delete window;
}

void GLwinEnableCustomTitleBar(GLWIN_window* window, int enable)
{
	// Placeholder implementation
	// Custom title bar implementation would go here
    if (!window || !window->hwnd) return;

    LONG style = GetWindowLongPtr(window->hwnd, GWL_STYLE);
    if (enable == GLWIN_TRUE) {
        // Hide default title bar and borders
        style &= ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    }
    else {
        // Restore default style
        style |= (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    }
    SetWindowLongPtr(window->hwnd, GWL_STYLE, style);
    SetWindowPos(window->hwnd, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}


void GLwinMakeContextCurrent(GLWIN_window* window) {
    if (!window || !window->hdc || !window->hglrc) return;
    wglMakeCurrent(window->hdc, window->hglrc);
}

void* GLwinGetProcAddress(const char* procname)
{
    void* p = (void*)wglGetProcAddress(procname);
    if (p == nullptr) {
        // Try to get from OpenGL32.dll for old functions
        static HMODULE module = LoadLibraryA("opengl32.dll");
        if (module)
            p = (void*)GetProcAddress(module, procname);
    }
    return p;
}

void GLwinSwapBuffers(GLWIN_window* window) {
    if (window && window->hdc) {
        ::SwapBuffers(window->hdc);
    }
}

void GLwinPollEvents(void) {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
bool GLwinWindowShouldClose(GLWIN_window* window, bool close) {
	if (window) {
		if (close) {
			window->closed = true;
		}
		return window->closed;
	}
	return true;
   // return window ? window->closed : 1;
}
void GLwinRestoreWindow(GLWIN_window* window)
{
	if (!window || !window->hwnd) return;
	ShowWindow(window->hwnd, SW_RESTORE);
}

void GLwinMinimizeWindow(GLWIN_window* window)
{
	if (!window || !window->hwnd) return;
	ShowWindow(window->hwnd, SW_MINIMIZE);
}

void GLwinMaximizeWindow(GLWIN_window* window)
{
	if (!window || !window->hwnd) return;
	ShowWindow(window->hwnd, SW_MAXIMIZE);
}


void GLwinGetFramebufferSize(GLWIN_window* window, int* width, int* height) {
    if (!window || !window->hwnd) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }
    RECT rect;
    if (GetClientRect(window->hwnd, &rect)) {
        if (width) *width = rect.right - rect.left;
        if (height) *height = rect.bottom - rect.top;
    }
    else {
        if (width) *width = 0;
        if (height) *height = 0;
    }
}

void GLwinGetWindowPos(GLWIN_window* window, int* winX, int* winY)
{
	// Get window position
	if (!window || !window->hwnd) {
		if (winX) *winX = 0;
		if (winY) *winY = 0;
		return;
	}
    RECT rect;
    if (GetWindowRect(window->hwnd, &rect)) {
        if (winX) *winX = rect.left;
        if (winY) *winY = rect.top;
        printf("DEBUG GetWindowRect -> left=%d top=%d right=%d bottom=%d\n",
            rect.left, rect.top, rect.right, rect.bottom);
    }
    else {
        if (winX) *winX = 0;
        if (winY) *winY = 0;
    }

}

void GLwinSetWindowPos(GLWIN_window* window, int posX, int posY)
{
	// Set window position
	if (!window || !window->hwnd) return;
    SetWindowPos(window->hwnd, nullptr, posX, posY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);


}

int GLwinGetWidth(GLWIN_window* window) {
    int w = 0;
    GLwinGetFramebufferSize(window, &w, nullptr);
    return w;
}

int GLwinGetHeight(GLWIN_window* window) {
    int h = 0;
    GLwinGetFramebufferSize(window, nullptr, &h);
    return h;
}

void GLwinSetWindowIcon(GLWIN_window* window, const wchar_t* iconPath) {
    if (!window || !window->hwnd) return;
    HICON hIcon = (HICON)LoadImage(
        nullptr, iconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE
    );
    if (hIcon) {
        SendMessage(window->hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(window->hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }
}

void GLwinGetTimer(GLWIN_window* window, int tstart, int tmax)
{
	if (!window) return;
	// Placeholder implementation
	// You can implement a proper timer using QueryPerformanceCounter or timeGetTime
}





//bool GLwinSetScreenMaximized(GLWIN_window* window, bool maximize) {
//    if (!window || !window->hwnd) return false;
//    ShowWindow(window->hwnd, maximize ? SW_MAXIMIZE : SW_RESTORE);
//    WINDOWPLACEMENT wp;
//    wp.length = sizeof(WINDOWPLACEMENT);
//    if (GetWindowPlacement(window->hwnd, &wp)) {
//        return (maximize ? wp.showCmd == SW_MAXIMIZE
//            : wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_RESTORE);
//    }
//    return false;
//}

void GLwinWindowHint(int hint, int value) {

    switch (hint)
    {
    case GLWIN_MAXIMIZED:
        std::cout << "GLwinWindowHint: GLWIN_MAXIMIZED hint set to " << value << " (implemented)\n";
        g_GLwinMaximizedHint = value;
        break;
    case GLWIN_RESIZABLE:
        std::cout << "GLwinWindowHint: GLWIN_RESIZABLE hint set to " << value << " (implemented)\n";
        g_GLwinResizableHint = value;
        break;
    default:
        break;
    }
}


// Keyboard state 
int GLwinGetKey(GLWIN_window* window, int keycode) {
    if (!window) return GLWIN_RELEASE;
    auto it = window->keyState.find(keycode);
    return (it != window->keyState.end() && it->second) ? GLWIN_PRESS : GLWIN_RELEASE;
}

// Set key callback (only key state tracking implemented)
void GLwinSetKeyCallback(GLWIN_window* window, GLwinKeyCallback callback)
{
	if (window) window->keyCallback = callback;
}


// Set character callback (implemented)
void GLwinSetCharCallback(GLWIN_window* window, GLwinCharCallback callback)
{
	if (!window) return;
	window->charCallback = callback;

}

// Mouse callback setters (new implementations)
void GLwinSetMouseButtonCallback(GLWIN_window* window, GLwinMouseButtonCallback cb)
{
    if (!window) return;
    window->mouseButtonCallback = cb;
}

void GLwinSetCursorPosCallback(GLWIN_window* window, GLwinCursorPosCallback cb)
{
    if (!window) return;
    window->cursorPosCallback = cb;
}

void GLwinSetScrollCallback(GLWIN_window* window, GLwinScrollCallback cb)
{
    if (!window) return;
    window->scrollCallback = cb;
}

// Mouse state
int GLwinGetMouseButton(GLWIN_window* window, int button)
{
    if (!window || button < 0 || button > 2) return GLWIN_RELEASE;
    return window->mouseButtons[button] ? GLWIN_PRESS : GLWIN_RELEASE;
}

// Get global cursor position
void GLwinGetGlobalCursorPos(GLWIN_window* window, int* x, int* y)
{
    POINT p;
    if (GetCursorPos(&p)) {
        if (x) *x = p.x;
        if (y) *y = p.y;
	}
    else {
        if (x) *x = 0;
        if (y) *y = 0;
    }
}
// Get client area screen origin
void GLwinGetClientScreenOrigin(GLWIN_window* window, int* outX, int* outY)
{
    if (!window || !window->hwnd) {
        if (outX) *outX = 0;
        if (outY) *outY = 0;
        return;
    }
    POINT pt = { 0, 0 };
    if (ClientToScreen(window->hwnd, &pt)) {
        if (outX) *outX = pt.x;
        if (outY) *outY = pt.y;
    }
    else {
        if (outX) *outX = 0;
        if (outY) *outY = 0;
    }
}




// Get cursor position
void GLwinGetCursorPos(GLWIN_window* window, double* xpos, double* ypos)
{
    if (!window) {
        if (xpos) *xpos = 0;
        if (ypos) *ypos = 0;
        return;
    }
    if (xpos) *xpos = window->mouseX;
    if (ypos) *ypos = window->mouseY;
}



void GLwinSetWindowTitle(GLWIN_window* window, const wchar_t* title)
{
	if (!window || !window->hwnd) return;
	SetWindowText(window->hwnd, title);
}

// Optional: terminate function
void GLwinTerminate(void) {
    // For now, nothing (all handled per-window)
}

// Window procedure (handles messages and input)
static LRESULT CALLBACK GLwin_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GLWIN_window* window = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = static_cast<GLWIN_window*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        if (window) window->hwnd = hwnd;
    }
    else {
        window = reinterpret_cast<GLWIN_window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    switch (msg) {
    case WM_CLOSE:
        if (window) window->closed = true;
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        if (window) {
            window->width = LOWORD(lParam);
            window->height = HIWORD(lParam);
            // Recreate backbuffer on resize (if present)
            if (window->backBitmap) {
                // Destroy and recreate with new size
                GLwinDestroyBackbuffer(window);
                glwin_internal_create_backbuffer(window, window->width, window->height);
            }
            if (window->resizeCallback) {
                window->resizeCallback(window->width, window->height);
            }
        }
        return 0;
    case WM_KEYDOWN:
        if (window) {
            window->keyState[(int)wParam] = true;
            if (window->keyCallback)
                window->keyCallback((int)wParam, GLWIN_PRESS);
        }
        return 0;
    case WM_KEYUP:
        if (window) {
            window->keyState[(int)wParam] = false;
            /*if (window->keyCallback)
                window->keyCallback((int)wParam, GLWIN_PRESS);*/
        }
        return 0;
        
		// Character input
    case WM_CHAR:
        if (window && window->charCallback) {
            window->charCallback((unsigned int)wParam);
        }
        break;

        // Mouse events can be added here
    case WM_MOUSEMOVE:
        if (window) {
            window->mouseX = GET_X_LPARAM(lParam);
            window->mouseY = GET_Y_LPARAM(lParam);
        }
        break;
    case WM_LBUTTONDOWN:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_LEFT] = true;
        break;
    case WM_LBUTTONUP:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_LEFT] = false;
        break;
    case WM_RBUTTONDOWN:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_RIGHT] = true;
        break;
    case WM_RBUTTONUP:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_RIGHT] = false;
        break;
    case WM_MBUTTONDOWN:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_MIDDLE] = true;
        break;
    case WM_MBUTTONUP:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_MIDDLE] = false;
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

