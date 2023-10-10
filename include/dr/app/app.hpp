#pragma once

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_gl.h>
#include <sokol_imgui.h>

#include <dr/basic_types.hpp>

namespace dr
{

struct App
{
    using Desc = sapp_desc;

    struct Scene
    {
        char const* name;
        void (*open)();
        void (*close)();
        void (*update)();
        void (*draw)();
        void (*handle_event)(sapp_event const*);
    };

    struct Config
    {
        sg_desc (*gfx_desc)();
        sgl_desc_t (*gl_desc)();
        simgui_desc_t (*ui_desc)();
        sg_pass_action pass_action;
    };
};

App::Desc app_desc();

App::Scene const* app_scene();

void app_set_scene(App::Scene const* scene);

App::Config& app_config();

u64 app_time();
u64 app_time_s();
u64 app_time_ms();

u64 app_delta_time();
f64 app_delta_time_s();
f64 app_delta_time_ms();

f32 app_aspect();

} // namespace dr
