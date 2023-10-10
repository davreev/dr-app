#include <dr/app/app.hpp>

#include <sokol_gl.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>
#include <sokol_time.h>

#include <fmt/core.h>

#include <dr/app/shim/imgui.hpp>
#include <dr/app/thread_pool.hpp>

#include "app.h"

namespace dr
{
namespace
{

App::Scene const* default_scene();

App::Config default_config();

struct
{
    App::Scene const* scene{default_scene()};
    App::Config config{default_config()};
    u64 time{};
    u64 delta_time{};
    bool is_init{};
} state;

App::Scene const* default_scene()
{
    auto const draw = []() {
        ImGui::BeginTooltip();
        ImGui::Text("Did you forget to assign a scene?");
        ImGui::EndTooltip();
    };

    static App::Scene const scene{
        "Default Scene",
        nullptr,
        nullptr,
        nullptr,
        draw,
        nullptr,
    };

    return &scene;
}

App::Config default_config()
{
    auto default_gfx_desc = []() { return sg_desc{}; };
    auto default_ui_desc = []() { return simgui_desc_t{}; };

    return App::Config{
        default_gfx_desc,
        default_gl_desc,
        default_ui_desc,
        default_pass_action(),
    };
}

void log(char const* const message, void* /*user_data*/) { fmt::print("{}\n", message); }

void init()
{
    auto gfx_desc = state.config.gfx_desc();
    {
        gfx_desc.context = sapp_sgcontext();
        gfx_desc.logger = {log, nullptr};
        // ...
    }
    sg_setup(gfx_desc);

    auto gl_desc = state.config.gl_desc();
    {
        gl_desc.logger = {log, nullptr};
        // ...
    }
    sgl_setup(gl_desc);

    auto ui_desc = state.config.ui_desc();
    {
        ui_desc.sample_count = sapp_sample_count();
        // ...
    }
    simgui_setup(ui_desc);

    stm_setup();

    ImGuiStyles::set_default(ImGui::GetStyle());

    if (state.scene->open)
        state.scene->open();

    state.is_init = true;
}

void frame()
{
    state.delta_time = stm_laptime(&state.time);

    if (state.scene->update)
        state.scene->update();

    // Main render pass
    {
        simgui_new_frame({
            sapp_width(),
            sapp_height(),
            stm_sec(state.delta_time),
            sapp_dpi_scale(),
        });

        sg_begin_default_pass(state.config.pass_action, sapp_width(), sapp_height());

        if (state.scene->draw)
            state.scene->draw();

        simgui_render();
        sg_end_pass();
        sg_commit();
    }
}

void cleanup()
{
    if (state.scene->close)
        state.scene->close();

    simgui_shutdown();
    sgl_shutdown();
    sg_shutdown();
}

void event(sapp_event const* const event)
{
    // NOTE: Touch begin events aren't properly consumed by the UI event handler so they're always
    // forwarded. Specifically, the begin event of the first UI touch *isn't* consumed by the UI
    // handler and the begin event of the first non-UI touch *is* consumed the by UI handler.

    if (!simgui_handle_event(event) || (event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN))
    {
        if (state.scene->handle_event)
            state.scene->handle_event(event);
    }
}

} // namespace

sapp_desc app_desc()
{
    auto desc = default_app_desc();
    {
        desc.init_cb = init;
        desc.frame_cb = frame;
        desc.cleanup_cb = cleanup;
        desc.event_cb = event;
        desc.logger = {log, nullptr};
    }
    return desc;
}

App::Scene const* app_scene() { return state.scene; }

void app_set_scene(App::Scene const* const scene)
{
    if (state.is_init)
    {
        if (state.scene->close)
            state.scene->close();

        if (scene->open)
            scene->open();
    }

    state.scene = scene;
}

App::Config& app_config() { return state.config; }

u64 app_time() { return state.time; }
u64 app_time_s() { return stm_sec(state.time); }
u64 app_time_ms() { return stm_ms(state.time); }

u64 app_delta_time() { return state.delta_time; }
f64 app_delta_time_s() { return stm_sec(state.delta_time); }
f64 app_delta_time_ms() { return stm_ms(state.delta_time); }

f32 app_aspect() { return sapp_widthf() / sapp_heightf(); }

} // namespace dr