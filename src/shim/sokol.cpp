// NOTE(dr): ImGui must be included before sokol_app.h on Linux builds to avoid clashes with X11
// macros from Xlib.h
#include <imgui.h>

#define SOKOL_NO_ENTRY
#define SOKOL_APP_IMPL
#include <sokol_app.h>

#define SOKOL_GFX_IMPL
#include <sokol_gfx.h>

#define SOKOL_GL_IMPL
#include <sokol_gl.h>

#define SOKOL_IMGUI_IMPL
#include <sokol_imgui.h>

#define SOKOL_TIME_IMPL
#include <sokol_time.h>

#define SOKOL_GLUE_IMPL
#include <sokol_glue.h>

#define SOKOL_LOG_IMPL
#include <sokol_log.h>