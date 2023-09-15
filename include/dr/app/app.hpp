#pragma once

#include <sokol_app.h>
#include <sokol_gfx.h>

#include <dr/basic_types.hpp>

namespace dr
{

struct Scene
{
    using Callback = void();
    using EventCallback = void(sapp_event const*);

    char const* name;
    Callback* open;
    Callback* update;
    Callback* draw;
    Callback* close;
    EventCallback* input;
};

sapp_desc app_desc();

Scene const* app_scene();

bool app_set_scene(Scene const* scene);

u64 app_time();
u64 app_time_s();
u64 app_time_ms();

u64 app_delta_time();
f64 app_delta_time_s();
f64 app_delta_time_ms();

f32 app_aspect();

sg_pass_action& app_pass_action();

} // namespace dr
