#ifndef APP_CONFIG_INCLUDED
#define APP_CONFIG_INCLUDED

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_gl.h>
#include <sokol_imgui.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*AppCallback)(void);
typedef void (*AppEventCallback)(sapp_event const*);

sapp_desc default_app_desc(
    AppCallback init, 
    AppCallback frame, 
    AppCallback cleanup, 
    AppEventCallback input,
    sapp_logger logger);

sg_desc default_gfx_desc(sg_logger logger);

sgl_desc_t default_gl_desc(sgl_logger_t logger);

simgui_desc_t default_ui_desc(int sample_count);

sg_pass_action default_pass_action(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // APP_CONFIG_INCLUDED
