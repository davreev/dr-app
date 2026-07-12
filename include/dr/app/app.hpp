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
        void (*handle_event)(Event const&);
        void* userdata;
    };

    struct Desc
    {
        Scene scene = App::default_scene();
        void (*init_cb)();
        void (*deinit_cb)();
        struct
        {
            i32 width = 800;
            i32 height = 600;
            char const* title;
        } window;
        struct
        {
            void (*app)(sapp_desc&);
            void (*gfx)(sg_desc&);
            void (*gl)(sgl_desc_t&);
            void (*imgui)(simgui_desc_t&);
        } sokol_config;
    };

    static void run(Desc const& desc);

    static Scene const& scene();
    static void set_scene(Scene const& scene);
    static Scene default_scene();

    static void begin_swapchain_pass(sg_pass_action const& action);
    static void begin_swapchain_pass();
    static void end_swapchain_pass();

    struct Input
    {
        f32 prev_touch_points[SAPP_MAX_TOUCHPOINTS][2]{};
        i32 prev_num_touches{};
        bool mouse_down[SAPP_MAX_MOUSEBUTTONS]{};
    };

    static Input const& input();

    static i32 framebuffer_width();
    static i32 framebuffer_height();
    static i32 window_width();
    static i32 window_height();
    static f32 aspect();

    static u64 time();
    static f64 time_s();
    static f64 time_ms();

    static u64 delta_time();
    static f64 delta_time_s();
    static f64 delta_time_ms();

    struct Profiler
    {
        static constexpr u64 frame_interval = 60;
        u64 frame_count{};
        u64 start_time{};
        u64 elapsed_time{};

        void start();
        void update();

        u64 frame_duration() const;
        f64 frame_duration_s() const;
        f64 frame_duration_ms() const;
        f64 fps() const;
    };

    static Profiler& profiler();
};

} // namespace dr
