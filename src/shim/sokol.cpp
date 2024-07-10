#ifdef __EMSCRIPTEN__
#define SOKOL_GLES3
#else
#define SOKOL_GLCORE
#endif

#define SOKOL_APP_IMPL
#include <sokol_app.h>

#define SOKOL_GFX_IMPL
#include <sokol_gfx.h>

#define SOKOL_GL_IMPL
#include <sokol_gl.h>

#define SOKOL_IMGUI_IMPL
#include <imgui.h>
#include <sokol_imgui.h>

#define SOKOL_TIME_IMPL
#include <sokol_time.h>

#define SOKOL_GLUE_IMPL
#include <sokol_glue.h>

#define SOKOL_LOG_IMPL
#include <sokol_log.h>