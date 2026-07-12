#if defined(TRACY_ENABLE) && !defined(__APPLE__) && !defined(__EMSCRIPTEN__)
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>
