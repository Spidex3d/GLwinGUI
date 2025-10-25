#include "../GLwin.h"
#include <iostream>

#include "../GLwinLog.h"

#include <windows.h>

#include <vector>
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
    GLwinCursorPosCallback   cursorPosCallback = nullptr;
    GLwinScrollCallback      scrollCallback = nullptr;

    // Drop callback
    GLwinDropCallback        dropCallback = nullptr;

    // Clipboard cached UTF-8 string (returned pointer from GLwinGetClipboardString)
    std::string              clipboardString;

	void* userPointer = nullptr; // for user data

    // Cursor visible state cache (keeps track of desired visibility)
    bool cursorVisible = true;

	
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

static std::wstring utf8_to_wstring(const std::string& utf8) {
    if (utf8.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
    if (size_needed <= 0) return std::wstring();
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wstr[0], size_needed);
    return wstr;
}


static std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    if (size_needed <= 0) return std::string();
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}

// Note: g_GLwinSwapInterval is defined/cached in your GLwin_set_swap implementation file.
extern int g_GLwinSwapInterval; // 0 = no vsync, 1 = vsync, etc.

static LARGE_INTEGER g_perfFreq;
static LARGE_INTEGER g_lastPresentTime;
static int g_vsyncInited = 0;

static void vsync_init_if_needed() {
    if (!g_vsyncInited) {
        QueryPerformanceFrequency(&g_perfFreq);
        QueryPerformanceCounter(&g_lastPresentTime);
        g_vsyncInited = 1;
    }
}
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
static int g_GLwinSwapInterval = 0;

// typedef for wglSwapIntervalEXT
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXT)(int interval);

// cached function pointer
static PFNWGLSWAPINTERVALEXT g_wglSwapIntervalEXT = nullptr;

int GLwinSetSwapInterval(int interval)
{
    if (interval < 0) interval = 0; // clamp negative values

    int prev = g_GLwinSwapInterval;
    g_GLwinSwapInterval = interval;

    // Try to get and cache wglSwapIntervalEXT. This requires a current context on the calling thread.
    if (!g_wglSwapIntervalEXT) {
        void* proc = (void*)wglGetProcAddress("wglSwapIntervalEXT");
        if (proc) g_wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXT)proc;
    }

    if (g_wglSwapIntervalEXT) {
        // call extension (no harm if it fails)
        g_wglSwapIntervalEXT(g_GLwinSwapInterval);
    }

    return prev;

}

void GLwinApplySwapIntervalSleep(GLWIN_window* window)
{
    if (!window) return;
    vsync_init_if_needed();

    if (g_GLwinSwapInterval <= 0) {
        QueryPerformanceCounter(&g_lastPresentTime);
        return;
    }

    int refreshHz = GLwinGetRefreshRate(window);
    if (refreshHz <= 0) {
        // unknown refresh rate => update lastPresent and return
        QueryPerformanceCounter(&g_lastPresentTime);
        return;
    }

    // desired interval in seconds = interval / refreshHz
    double desiredSec = (double)g_GLwinSwapInterval / (double)refreshHz;
    double desiredTicks = desiredSec * (double)g_perfFreq.QuadPart;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    double last = (double)g_lastPresentTime.QuadPart;
    double target = last + desiredTicks;
    double nowd = (double)now.QuadPart;

    if (nowd >= target) {
        // we're late or on time - update lastPresent and return
        g_lastPresentTime = now;
        return;
    }

    double remainingTicks = target - nowd;
    double remainingMs = (remainingTicks * 1000.0) / (double)g_perfFreq.QuadPart;

    if (remainingMs > 10.0) {
        // sleep the bulk, leave a few ms for spin waiting
        DWORD sleepMs = (DWORD)(remainingMs - 5.0);
        Sleep(sleepMs);
    }

    // spin for remaining time for better precision
    do {
        QueryPerformanceCounter(&now);
    } while ((double)now.QuadPart < target);

    g_lastPresentTime = now;
}

int GLwinGetRefreshRate(GLWIN_window* window) {
    if (!window || !window->hwnd) return 0;

    // Get the nearest monitor for the window
    HMONITOR hMon = MonitorFromWindow(window->hwnd, MONITOR_DEFAULTTONEAREST);
    if (!hMon) return 0;

    MONITORINFOEX mi;
    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfo(hMon, &mi)) {
        // Couldn't get monitor info
        return 0;
    }

    // Primary method: EnumDisplaySettings for the monitor device name
    DEVMODE dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    if (EnumDisplaySettings(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm)) {
        int freq = (int)dm.dmDisplayFrequency;
        if (freq > 0) return freq;
    }

    // Fallback: GetDeviceCaps on the monitor's HDC
    HDC hdc = NULL;
    // Create a DC for the monitor device name (use CreateDC when device name available).
    if (mi.szDevice[0] != 0) {
        hdc = CreateDC(mi.szDevice, mi.szDevice, NULL, NULL);
    }
    if (!hdc) {
        // Fallback to the window DC for the window's screen
        hdc = GetDC(window->hwnd);
    }
    if (hdc) {
        int vrefresh = GetDeviceCaps(hdc, VREFRESH);
        if (hdc != GetDC(NULL)) { // if we created the DC using CreateDC, release it with DeleteDC
            if (mi.szDevice[0] != 0) DeleteDC(hdc);
        }
        else {
            ReleaseDC(window->hwnd, hdc);
        }
        if (vrefresh > 0) return vrefresh;
    }

    // Unknown
    return 0;
}

/* GLwinSetUserPointer & GLwinGetUserPointer they read/write the existing userPointer field already present in your internal GLWIN_window struct.
These are tiny, safe convenience helpers that let applications attach arbitrary data (e.g., a pointer to a scene,
application state, or C++ object) to a window.*/

void GLwinSetUserPointer(GLWIN_window* window, void* ptr)
{
	if (!window) return;
	window->userPointer = ptr;
}

void* GLwinGetUserPointer(GLWIN_window* window)
{
	if (!window) return nullptr;
    return nullptr;
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
    DragAcceptFiles(hwnd, TRUE); // darg adn drop

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

void GLwinSetCursorVisible(GLWIN_window* window, int visible)
{
    // If no window, just adjust global cursor visibility
    bool wantVisible = (visible != 0);
    if (!window) {
        // Use ShowCursor until it reaches the desired state
        if (wantVisible) {
            int cnt = ShowCursor(TRUE);
            while (cnt < 0) cnt = ShowCursor(TRUE);
        }
        else {
            int cnt = ShowCursor(FALSE);
            while (cnt >= 0) cnt = ShowCursor(FALSE);
        }
        return;
    }

    window->cursorVisible = wantVisible;
    if (wantVisible) {
        int cnt = ShowCursor(TRUE);
        while (cnt < 0) cnt = ShowCursor(TRUE);
    }
    else {
        int cnt = ShowCursor(FALSE);
        while (cnt >= 0) cnt = ShowCursor(FALSE);
    }
}

// New: Set the cursor position in client coordinates for the given window.
// Converts client coords to screen coords and calls SetCursorPos. Updates internal mouseX/mouseY
// and invokes cursor callback if set.
void GLwinSetCursorPos(GLWIN_window* window, int x, int y)
{
    // If a window is provided, convert client -> screen coords
    if (window && window->hwnd) {
        POINT pt = { x, y };
        if (!ClientToScreen(window->hwnd, &pt)) {
            // fallback: just use x,y as screen coords
            SetCursorPos(x, y);
        }
        else {
            SetCursorPos(pt.x, pt.y);
        }
        // update cached client-space mouse position
        window->mouseX = (double)x;
        window->mouseY = (double)y;
        // fire cursor callback
        if (window->cursorPosCallback) {
            window->cursorPosCallback(window->mouseX, window->mouseY);
        }
    }
    else {
        // No window: interpret x,y as screen coordinates
        SetCursorPos(x, y);
    }
}

void GLwinSetDropCallback(GLWIN_window* window, GLwinDropCallback cb)
{
	if (!window) return;
	window->dropCallback = cb;
}

void GLwinSetClipboardString(GLWIN_window* window, const char* str)
{
    if (!str) return;

    // Convert UTF-8 input to UTF-16 (wide) for the clipboard
    std::string sutf8 = str;
    std::wstring wtext = utf8_to_wstring(sutf8);
    if (wtext.empty()) {
        // If conversion produced empty string, still try to clear clipboard
    }

    if (!OpenClipboard(window ? window->hwnd : NULL)) {
        return;
    }

    // Empty and set data
    EmptyClipboard();

    // Allocate global memory for Unicode text (wide chars + null)
    SIZE_T bytes = (wtext.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem) {
        CloseClipboard();
        return;
    }
    void* p = GlobalLock(hMem);
    if (p) {
        memcpy(p, wtext.c_str(), bytes);
        GlobalUnlock(hMem);
        // SetClipboardData takes ownership of hMem
        SetClipboardData(CF_UNICODETEXT, hMem);
    }
    else {
        GlobalFree(hMem);
    }

    CloseClipboard();
}

const char* GLwinGetClipboardString(GLWIN_window* window)
{
    if (!window) return nullptr;

    window->clipboardString.clear();

    if (!OpenClipboard(window->hwnd)) {
        // Return empty string pointer (internal storage)
        window->clipboardString.clear();
        return window->clipboardString.c_str();
    }

    if (!IsClipboardFormatAvailable(CF_UNICODETEXT) && !IsClipboardFormatAvailable(CF_TEXT)) {
        CloseClipboard();
        window->clipboardString.clear();
        return window->clipboardString.c_str();
    }

    // Prefer Unicode text
    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        HGLOBAL hData = GetClipboardData(CF_UNICODETEXT);
        if (hData) {
            LPCWSTR pdata = (LPCWSTR)GlobalLock(hData);
            if (pdata) {
                std::wstring wstr(pdata);
                window->clipboardString = wstring_to_utf8(wstr);
                GlobalUnlock(hData);
            }
        }
    }
    else {
        // Fallback to ANSI CF_TEXT
        HGLOBAL hData = GetClipboardData(CF_TEXT);
        if (hData) {
            LPCSTR pdata = (LPCSTR)GlobalLock(hData);
            if (pdata) {
                // Treat as ANSI, convert to UTF-8
                std::string ansi(pdata);
                // Convert ANSI to wide, then to UTF-8
                int needed = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), (int)ansi.size(), NULL, 0);
                if (needed > 0) {
                    std::wstring wstr(needed, 0);
                    MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), (int)ansi.size(), &wstr[0], needed);
                    window->clipboardString = wstring_to_utf8(wstr);
                }
                else {
                    window->clipboardString = ansi; // fallback
                }
                GlobalUnlock(hData);
            }
        }
    }

    CloseClipboard();
    return window->clipboardString.c_str();
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

// Helper: compute modifier flags for callbacks
// Bitmask layout returned in `mods` parameter:
// bit 0 (1) = SHIFT, bit 1 (2) = CTRL, bit 2 (4) = ALT
static int glwin_get_mods_from_key_state()
{
    int mods = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000) mods |= 1;
    if (GetKeyState(VK_CONTROL) & 0x8000) mods |= 2;
    if (GetKeyState(VK_MENU) & 0x8000) mods |= 4;
    return mods;
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
	case WM_DROPFILES:
        if (window && window->dropCallback) {
            HDROP hDrop = (HDROP)wParam;
            UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
            if (count > 0) {
                std::vector<std::wstring> paths;
                paths.reserve(count);
                for (UINT i = 0; i < count; ++i) {
                    UINT len = DragQueryFileW(hDrop, i, NULL, 0);
                    if (len == 0) {
                        paths.emplace_back();
                        continue;
                    }
                    std::wstring buf;
                    buf.resize(len + 1);
                    DragQueryFileW(hDrop, i, &buf[0], len + 1);
                    // ensure null-termination and trim to actual length
                    buf.resize(wcslen(buf.c_str()));
                    paths.push_back(std::move(buf));
                }
                // Build array of const wchar_t* for callback
                std::vector<const wchar_t*> ptrs;
                ptrs.reserve(paths.size());
                for (const auto& s : paths) ptrs.push_back(s.c_str());
                // Invoke callback
                window->dropCallback((int)ptrs.size(), ptrs.empty() ? nullptr : ptrs.data());
            }
            DragFinish(hDrop);
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
            if (window->cursorPosCallback) {
                // callback expects double xpos, double ypos
                window->cursorPosCallback(window->mouseX, window->mouseY);
            }
        }
        break;
    case WM_LBUTTONDOWN:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_LEFT] = true;
		if (window && window->mouseButtonCallback) {
			int mods = glwin_get_mods_from_key_state();
			window->mouseButtonCallback(GLWIN_MOUSE_BUTTON_LEFT, GLWIN_PRESS, mods);
		}
        break;
    case WM_LBUTTONUP:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_LEFT] = false;
		if (window->mouseButtonCallback) {
			int mods = glwin_get_mods_from_key_state();
			window->mouseButtonCallback(GLWIN_MOUSE_BUTTON_LEFT, GLWIN_RELEASE, mods);
		}
        break;
    case WM_RBUTTONDOWN:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_RIGHT] = true;
		if (window->mouseButtonCallback) {
			int mods = glwin_get_mods_from_key_state();
			window->mouseButtonCallback(GLWIN_MOUSE_BUTTON_RIGHT, GLWIN_PRESS, mods);
		}
        break;
    case WM_RBUTTONUP:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_RIGHT] = false;
		if (window->mouseButtonCallback) {
			int mods = glwin_get_mods_from_key_state();
			window->mouseButtonCallback(GLWIN_MOUSE_BUTTON_RIGHT, GLWIN_RELEASE, mods);
		}
        break;
    case WM_MBUTTONDOWN:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_MIDDLE] = true;
		if (window->mouseButtonCallback) {
			int mods = glwin_get_mods_from_key_state();
			window->mouseButtonCallback(GLWIN_MOUSE_BUTTON_MIDDLE, GLWIN_PRESS, mods);
		}
        break;
    case WM_MBUTTONUP:
        if (window) window->mouseButtons[GLWIN_MOUSE_BUTTON_MIDDLE] = false;
		if (window->mouseButtonCallback) {  
			int mods = glwin_get_mods_from_key_state(); 
			window->mouseButtonCallback(GLWIN_MOUSE_BUTTON_MIDDLE, GLWIN_RELEASE, mods);
		}
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

