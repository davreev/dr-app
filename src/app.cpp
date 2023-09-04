#include <dr/app/app.hpp>

#include <sokol_gl.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>
#include <sokol_time.h>

#include <fmt/core.h>

#include <dr/app/app_config.h>
#include <dr/app/shim/imgui.hpp>
#include <dr/app/thread_pool.hpp>

#include "default_scene.hpp"

namespace dr
{
namespace
{

struct
{
    Scene const* scene{default_scene()};
    sg_pass_action pass_action{};
    u64 time{};
    u64 delta_time{};
    bool is_init{};
} state;

void log(char const* const message, void* /*user_data*/)
{
    fmt::print("{}\n", message);
}

void init()
{
    // Init Sokol
    {
        // Sokol gfx
        sg_setup(default_gfx_desc({log, nullptr}));

        // Sokol time
        stm_setup();

        // Sokol GL
        sgl_setup(default_gl_desc({log, nullptr}));

        // Sokol imgui
        simgui_setup(default_ui_desc(sapp_sample_count()));
    }

    // Init ImGui
    ImGuiStyles::set_default(ImGui::GetStyle());

    // Set default pass action
    state.pass_action.colors[0] = {
        SG_ACTION_CLEAR,
        {0.15, 0.15, 0.15, 0.15},
    };

    state.scene->open();
    state.is_init = true;
}

void frame()
{
    state.delta_time = stm_laptime(&state.time);
    state.scene->update();

    // Main render pass
    {
        simgui_new_frame({
            sapp_width(),
            sapp_height(),
            stm_sec(state.delta_time),
            sapp_dpi_scale(),
        });

        sg_begin_default_pass(
            state.pass_action,
            sapp_width(),
            sapp_height());

        state.scene->draw();

        simgui_render();
        sg_end_pass();
        sg_commit();
    }
}

void cleanup()
{
    state.scene->close();
    simgui_shutdown();
    sgl_shutdown();
    sg_shutdown();
}

void input(sapp_event const* const event)
{
    // NOTE: Touch begin events aren't properly consumed by the UI event handler so they're always
    // forwarded. Specifically, the begin event of the first UI touch *isn't* consumed by the UI
    // handler and the begin event of the first non-UI touch *is* consumed the by UI handler.

    if (!simgui_handle_event(event) || (event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN))
        state.scene->input(event);
}

bool scene_is_valid(Scene const* const scene)
{
    return scene != nullptr
        && scene->open != nullptr
        && scene->update != nullptr
        && scene->draw != nullptr
        && scene->close != nullptr
        && scene->input != nullptr;
}

} // namespace

sapp_desc app_desc()
{
    return default_app_desc(init, frame, cleanup, input, {log, nullptr});
}

Scene const* app_scene() { return state.scene; }

bool app_set_scene(Scene const* const scene)
{
    if (!scene_is_valid(scene))
        return false;

    if (state.is_init)
    {
        state.scene->close();
        scene->open();
    }

    state.scene = scene;
    return true;
}

u64 app_time() { return state.time; }
u64 app_time_s() { return stm_sec(state.time); }
u64 app_time_ms() { return stm_ms(state.time); }

u64 app_delta_time() { return state.delta_time; }
f64 app_delta_time_s() { return stm_sec(state.delta_time); }
f64 app_delta_time_ms() { return stm_ms(state.delta_time); }

f32 app_aspect() { return sapp_widthf() / sapp_heightf(); }

sg_pass_action& app_pass_action() { return state.pass_action; }

} // namespace dr