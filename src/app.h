#ifndef APP_H
#define APP_H

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_gl.h>

#ifdef __cplusplus
extern "C"
{
#endif

sapp_desc default_app_desc(void);

sgl_desc_t default_gl_desc(void);

sg_pass_action default_pass_action(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // APP_H
