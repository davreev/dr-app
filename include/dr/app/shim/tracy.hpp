#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#elif !defined(__EMSCRIPTEN__)
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>
