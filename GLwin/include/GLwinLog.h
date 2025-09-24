#pragma once
#include <iostream>
#include <sstream>

enum GLWIN_LOG_LEVEL {
    GLWIN_LOG_LEVEL_INIT = 0,
    GLWIN_LOG_LEVEL_ERROR = 1,
    GLWIN_LOG_LEVEL_WARNING = 2,
    GLWIN_LOG_LEVEL_INFO = 3,
    GLWIN_LOG_LEVEL_TRACE = 4,
    GLWIN_LOG_LEVEL_DEBUG = 5
};

// ANSI color codes
#define GLWIN_COLOR_RESET   "\x1b[0m"
#define GLWIN_COLOR_RED     "\x1b[31m"
#define GLWIN_COLOR_YELLOW  "\x1b[33m"
#define GLWIN_COLOR_GREEN   "\x1b[32m"
#define GLWIN_COLOR_CYAN    "\x1b[36m"
#define GLWIN_COLOR_BLUE   "\x1b[34m"

#ifdef _DEBUG
#ifndef GLWIN_LOG_LEVEL_ACTIVE
#define GLWIN_LOG_LEVEL_ACTIVE GLWIN_LOG_LEVEL_DEBUG
#endif

//Select color based on level
#define GLWIN_LOG_COLOR(level) \
        ((level) == GLWIN_LOG_LEVEL_INIT ? GLWIN_COLOR_GREEN : \
         (level) == GLWIN_LOG_LEVEL_ERROR ? GLWIN_COLOR_RED : \
         (level) == GLWIN_LOG_LEVEL_WARNING ? GLWIN_COLOR_YELLOW : \
         (level) == GLWIN_LOG_LEVEL_INFO ? GLWIN_COLOR_GREEN : \
         (level) == GLWIN_LOG_LEVEL_TRACE ? GLWIN_COLOR_CYAN : \
         GLWIN_COLOR_BLUE)

// Internal macro for logging with level and color
#define GLWIN_LOG(level, level_name, ...) do { \
        if ((level) <= GLWIN_LOG_LEVEL_ACTIVE) { \
            std::ostringstream glwin_log_stream; \
            glwin_log_stream << __VA_ARGS__; \
            std::cout << GLWIN_LOG_COLOR(level) \
                      << "[GLWIN][" << level_name << "] " << glwin_log_stream.str() \
                      << GLWIN_COLOR_RESET << std::endl; \
        } \
    } while (0)


//#define GLWIN_LOG_INIT(x) std::cout << x << std::endl
#define GLWIN_LOG_INIT(...)   GLWIN_LOG(GLWIN_LOG_LEVEL_INIT, "INIT", __VA_ARGS__)
#define GLWIN_LOG_ERROR(...)   GLWIN_LOG(GLWIN_LOG_LEVEL_ERROR, "ERROR", __VA_ARGS__)
#define GLWIN_LOG_WARNING(...) GLWIN_LOG(GLWIN_LOG_LEVEL_WARNING, "WARN", __VA_ARGS__)
#define GLWIN_LOG_INFO(...)    GLWIN_LOG(GLWIN_LOG_LEVEL_INFO, "INFO", __VA_ARGS__)
#define GLWIN_LOG_TRACE(...)   GLWIN_LOG(GLWIN_LOG_LEVEL_TRACE, "TRACE", __VA_ARGS__)
#define GLWIN_LOG_DEBUG(...)   GLWIN_LOG(GLWIN_LOG_LEVEL_DEBUG, "DEBUG", __VA_ARGS__)

#else
// In release: macros do nothing
#define GLWIN_LOG_INIT(x)
#define GLWIN_LOG_ERROR(...)
#define GLWIN_LOG_WARNING(...)
#define GLWIN_LOG_INFO(...)
#define GLWIN_LOG_TRACE(...)
#define GLWIN_LOG_DEBUG(...)
#endif


