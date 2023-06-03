#include <dr/app/app_config.h>

#include <sokol_glue.h>

sapp_desc default_app_desc(
    AppCallback const init,
    AppCallback const frame,
    AppCallback const cleanup,
    AppEventCallback const input,
    sapp_logger const logger)
{
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = input,
        .sample_count = 2,
        .high_dpi = true,
        .enable_clipboard = true,
        .logger = logger,
        .win32_console_create = true,
        .win32_console_utf8 = true,
    };
}

sg_desc default_gfx_desc(sg_logger const logger)
{
    return (sg_desc){
        .logger = logger,
        .context = sapp_sgcontext(),
        // ...
    };
}

sgl_desc_t default_gl_desc(sgl_logger_t const logger)
{
    return (sgl_desc_t){
        .face_winding = SG_FACEWINDING_CCW,
        .logger = logger,
        // ...
    };
}

simgui_desc_t default_ui_desc(int const sample_count)
{
    return (simgui_desc_t){
        .sample_count = sample_count,
        // ...
    };
}
