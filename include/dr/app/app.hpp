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
    using Event = sapp_event;

    struct Scene
    {
        char const* name;
        void (*open)();
        void (*close)();
        void (*update)();
        void (*draw)();
        void (*handle_event)(Event const&);
        void* userdata;
    };

    struct Desc
    {
        Scene scene = default_scene();
        sg_pass_action pass_action = default_pass_action();
        void (*init_cb)();
        void (*deinit_cb)();
        struct
        {
            void (*app)(sapp_desc&);
            void (*gfx)(sg_desc&);
            void (*gl)(sgl_desc_t&);
            void (*imgui)(simgui_desc_t&);
        } sokol_config;
    };

    struct Input
    {
        f32 prev_touch_points[SAPP_MAX_TOUCHPOINTS][2]{};
        i32 prev_num_touches{};
        bool mouse_down[SAPP_MAX_MOUSEBUTTONS]{};
    };

    static Scene default_scene();
    static sg_pass_action default_pass_action();

    static void run(Desc const& desc);

    static Scene const& scene();
    static void set_scene(Scene const& scene);

    static Input const& input();

    static i32 width();
    static i32 height();
    static f32 aspect();

    static u64 time();
    static f64 time_s();
    static f64 time_ms();

    static u64 delta_time();
    static f64 delta_time_s();
    static f64 delta_time_ms();
};

} // namespace dr
