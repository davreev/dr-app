#include "app.h"

sapp_desc default_app_desc(void)
{
    return (sapp_desc){
        .sample_count = 4,
        .high_dpi = true,
        .enable_clipboard = true,
        .win32_console_utf8 = true,
        .win32_console_create = true,
        // ...
    };
}

sg_desc default_gfx_desc(void) { return (sg_desc){0}; }

sgl_desc_t default_gl_desc(void)
{
    return (sgl_desc_t){
        .face_winding = SG_FACEWINDING_CCW,
        // ...
    };
}

simgui_desc_t default_imgui_desc(void) { return (simgui_desc_t){0}; }

sg_pass_action default_pass_action(void)
{
    return (sg_pass_action){
        .colors[0] =
            {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = {0.15, 0.15, 0.15, 1.0},
            },
        // ...
    };
}
