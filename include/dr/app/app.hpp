#pragma once

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_gl.h>
#include <sokol_imgui.h>

#include <dr/basic_types.hpp>

#define DR_APP_MAIN sokol_main

namespace dr
{

struct App
{
    using Desc = sapp_desc;
    using Event = sapp_event;

    struct Scene
    {
        char const* name;
        void (*open)(void*);
        void (*close)(void*);
        void (*update)(void*);
        void (*draw)(void*);
        void (*handle_event)(void*, Event const&);
        void* context;
    };

    struct Config
    {
        struct InitContext
        {
            sg_desc gfx_desc;
            sgl_desc_t gl_desc;
            simgui_desc_t imgui_desc;
        };

        struct
        {
            void (*override)(InitContext& ctx);
            void (*callback)(void*);
        } init;

        struct
        {
            void (*callback)(void*);
        } cleanup;

        sg_pass_action pass_action;
        void* context;
    };

    static Desc desc();

    static Scene const& scene();
    static void set_scene(App::Scene const& scene);

    static Config& config();

    static u64 time();
    static f64 time_s();
    static f64 time_ms();

    static u64 delta_time();
    static f64 delta_time_s();
    static f64 delta_time_ms();

    static f32 aspect();
};

} // namespace dr
