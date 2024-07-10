#include <dr/app/app.hpp>

#include <cassert>

#include <sokol_gl.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>
#include <sokol_log.h>
#include <sokol_time.h>

#include <dr/app/shim/imgui.hpp>

#include "app.h"

namespace dr
{
namespace
{

App::Scene default_scene();
App::Config default_config();

struct
{
    App::Scene scene{default_scene()};
    App::Config config{default_config()};
    u64 time{};
    u64 delta_time{};
    bool is_init{};
} state;

App::Scene default_scene()
{
    return {
        "Default Scene",
        nullptr,
        nullptr,
        nullptr,
        [](void* /*context*/) {
            ImGui::BeginTooltip();
            ImGui::Text("This is the default scene. Nothing to see here.");
            ImGui::EndTooltip();
        },
        nullptr,
        nullptr,
    };
}

App::Config default_config() { return {{}, {}, default_pass_action(), nullptr}; }

void init()
{
    auto const& config = state.config;

    // Init sokol_gfx
    auto gfx_desc = sg_desc{};
    {
        gfx_desc.environment = sglue_environment();
        gfx_desc.logger.func = slog_func;
        // ...

        if (config.init.override_gfx)
            config.init.override_gfx(gfx_desc);
    }
    sg_setup(gfx_desc);

    // Init sokol_gl
    auto gl_desc = default_gl_desc();
    {
        gl_desc.logger.func = slog_func;
        // ...

        if (config.init.override_gl)
            config.init.override_gl(gl_desc);
    }
    sgl_setup(gl_desc);

    // Init sokol_imgui
    auto imgui_desc = simgui_desc_t{};
    {
        imgui_desc.sample_count = sapp_sample_count();
        // ...

        if (config.init.override_imgui)
            config.init.override_imgui(imgui_desc);
    }
    simgui_setup(imgui_desc);

    // Init sokol_time
    stm_setup();

    ImGuiStyles::set_default(ImGui::GetStyle());

    if (config.init.callback)
        config.init.callback(config.context);

    if (state.scene.open)
        state.scene.open(state.scene.context);

    state.is_init = true;
}

void frame()
{
    state.delta_time = stm_laptime(&state.time);

    if (state.scene.update)
        state.scene.update(state.scene.context);

    // Main render pass
    {
        simgui_new_frame({
            sapp_width(),
            sapp_height(),
            stm_sec(state.delta_time),
            sapp_dpi_scale(),
        });

        sg_pass pass{};
        {
            pass.action = state.config.pass_action;
            pass.swapchain = sglue_swapchain();
        }
        sg_begin_pass(&pass);

        if (state.scene.draw)
            state.scene.draw(state.scene.context);

        simgui_render();
        sg_end_pass();
        sg_commit();
    }
}

void cleanup()
{
    auto const& config = state.config;

    if (state.scene.close)
        state.scene.close(state.scene.context);

    if (config.cleanup.callback)
        config.cleanup.callback(config.context);

    simgui_shutdown();
    sgl_shutdown();
    sg_shutdown();
}

void event(App::Event const* const event)
{
    // NOTE: Touch begin events aren't properly consumed by simgui_handle_event so they're always
    // forwarded. Specifically, the begin event of the first UI touch *isn't* consumed and the begin
    // event of the first non-UI touch *is* consumed.

    if (!simgui_handle_event(event) || (event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN))
    {
        if (state.scene.handle_event)
            state.scene.handle_event(state.scene.context, *event);
    }
}

} // namespace

sapp_desc App::desc()
{
    auto desc = default_app_desc();
    {
        desc.init_cb = init;
        desc.frame_cb = frame;
        desc.cleanup_cb = cleanup;
        desc.event_cb = event;
        desc.logger.func = slog_func;
    }
    return desc;
}

App::Scene const& App::scene() { return state.scene; }

void App::set_scene(App::Scene const& scene)
{
    if (state.is_init)
    {
        if (state.scene.close)
            state.scene.close(state.scene.context);

        if (scene.open)
            scene.open(state.scene.context);
    }

    state.scene = scene;
}

App::Config& App::config() { return state.config; }

u64 App::time() { return state.time; }
f64 App::time_s() { return stm_sec(state.time); }
f64 App::time_ms() { return stm_ms(state.time); }

u64 App::delta_time() { return state.delta_time; }
f64 App::delta_time_s() { return stm_sec(state.delta_time); }
f64 App::delta_time_ms() { return stm_ms(state.delta_time); }

f32 App::aspect() { return sapp_widthf() / sapp_heightf(); }

} // namespace dr