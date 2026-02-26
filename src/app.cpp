#include <dr/app/app.hpp>

#include <sokol_glue.h>
#include <sokol_log.h>
#include <sokol_time.h>

#include <dr/app/shim/imgui.hpp>

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
        .name = "Default Scene",
        .draw =
            []() {
                ImGui::BeginTooltip();
                ImGui::Text("This is the default scene. Nothing to see here.");
                ImGui::EndTooltip();
            },
    };
}

App::Config default_config()
{
    return {
        .pass_action{
            .colors{
                {
                    .load_action = SG_LOADACTION_CLEAR,
                    .clear_value{0.15f, 0.15f, 0.15f, 1.0f},
                },
            },
        },
    };
}

void init()
{
    auto const& config = state.config;

    App::Config::InitContext ctx{
        .gfx_desc{
            .logger{.func = slog_func},
            .environment = sglue_environment(),
        },
        .gl_desc{
            .sample_count = sapp_sample_count(),
            .face_winding = SG_FACEWINDING_CCW,
            .logger{.func = slog_func},
        },
        .imgui_desc{
            .sample_count = sapp_sample_count(),
        },
    };

    if (config.init.override)
        config.init.override(ctx);

    sg_setup(ctx.gfx_desc);
    sgl_setup(ctx.gl_desc);
    simgui_setup(ctx.imgui_desc);
    stm_setup();

    ImGuiStyles::set_default(ImGui::GetStyle());

    if (config.init.callback)
        config.init.callback();

    if (state.scene.open)
        state.scene.open();

    state.is_init = true;
}

void frame()
{
    state.delta_time = stm_laptime(&state.time);

    if (state.scene.update)
        state.scene.update();

    // Main render pass
    {
        simgui_new_frame({
            .width = sapp_width(),
            .height = sapp_height(),
            .delta_time = stm_sec(state.delta_time),
            .dpi_scale = sapp_dpi_scale(),
        });

        sg_pass const pass{
            .action = state.config.pass_action,
            .swapchain = sglue_swapchain(),
        };
        sg_begin_pass(&pass);

        if (state.scene.draw)
            state.scene.draw();

        simgui_render();
        sg_end_pass();
        sg_commit();
    }
}

void cleanup()
{
    auto const& config = state.config;

    if (state.scene.close)
        state.scene.close();

    if (config.deinit.callback)
        config.deinit.callback();

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
            state.scene.handle_event(*event);
    }
}

} // namespace

App::Desc App::desc()
{
    return {
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .sample_count = 4,
        .high_dpi = true,
        .enable_clipboard = true,
        .logger{.func = slog_func},
        .win32_console_utf8 = true,
        .win32_console_create = true,
    };
}

void App::run(Desc const& desc) { sapp_run(desc); }

App::Scene const& App::scene() { return state.scene; }

void App::set_scene(App::Scene const& scene)
{
    if (state.is_init)
    {
        if (state.scene.close)
            state.scene.close();

        if (scene.open)
            scene.open();
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