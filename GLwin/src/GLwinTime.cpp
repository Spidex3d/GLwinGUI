#include "../GLwinTime.h"
#include <windows.h>

double GLwinGetTime(void) {
    static LARGE_INTEGER freq = {};
    static LARGE_INTEGER start = {};
    if (!freq.QuadPart) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - start.QuadPart) / (double)freq.QuadPart;
}