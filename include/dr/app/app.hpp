#pragma once

#include <sokol_app.h>

#include <dr/basic_types.hpp>

namespace dr
{

struct Scene
{
    using Callback = void (*)();
    using EventCallback = void (*)(sapp_event const*);

    char const* name;
    Callback open;
    Callback update;
    Callback draw;
    Callback close;
    EventCallback input;
};

sapp_desc app_desc();

bool app_init(Scene const* scene, isize num_worker_threads = 0);

Scene const* app_scene();

bool app_set_scene(Scene const* scene);

u64 app_time();

u64 app_delta_time();

f32 app_aspect();

} // namespace dr
