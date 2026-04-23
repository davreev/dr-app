#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#elif !defined(__EMSCRIPTEN__)
#if defined(_WIN32)
#include <windows.h>
#endif
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>
