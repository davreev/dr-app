#include "app.h"

sapp_desc default_app_desc(void)
{
    return (sapp_desc){
        .sample_count = 2,
        .high_dpi = true,
        .enable_clipboard = true,
        .win32_console_utf8 = true,
        .win32_console_create = true,
        // ...
    };
}

sgl_desc_t default_gl_desc(void)
{
    return (sgl_desc_t){
        .face_winding = SG_FACEWINDING_CCW,
        // ...
    };
}

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
