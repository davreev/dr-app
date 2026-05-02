#include <dr/app/app.hpp>

#include <sokol_glue.h>
#include <sokol_log.h>
#include <sokol_time.h>

#include <dr/app/shim/imgui.hpp>
#include <dr/app/shim/tracy.hpp>

namespace dr
{
namespace
{

struct
{
    App::Desc desc{};
    App::Scene next_scene{};
    App::Input input{};
    u64 time{};
    u64 delta_time{};
    bool scene_dirty{};
} state;

sg_desc sokol_gfx_desc()
{
    sg_desc desc{
        .logger{.func = slog_func},
        .environment = sglue_environment(),
    };

    // Apply overrides
    if (state.desc.sokol_config.gfx)
        state.desc.sokol_config.gfx(desc);

    return desc;
}

sgl_desc_t sokol_gl_desc()
{
    sgl_desc_t desc{
        .sample_count = sapp_sample_count(),
        .face_winding = SG_FACEWINDING_CCW,
        .logger{.func = slog_func},
    };

    // Apply overrides
    if (state.desc.sokol_config.gl)
        state.desc.sokol_config.gl(desc);

    return desc;
}

simgui_desc_t sokol_imgui_desc()
{
    simgui_desc_t desc{
        .sample_count = sapp_sample_count(),
    };

    // Apply overrides
    if (state.desc.sokol_config.imgui)
        state.desc.sokol_config.imgui(desc);

    return desc;
}

void init()
{
    sg_setup(sokol_gfx_desc());
    sgl_setup(sokol_gl_desc());
    simgui_setup(sokol_imgui_desc());
    stm_setup();

    ImGuiStyles::set_default(ImGui::GetStyle());

    TracyGpuContext;

    if (state.desc.init_cb)
        state.desc.init_cb();

    if (state.desc.scene.open)
        state.desc.scene.open();
}

void change_scene()
{
    if (state.desc.scene.close)
        state.desc.scene.close();

    if (state.next_scene.open)
        state.next_scene.open();

    state.desc.scene = state.next_scene;
    state.next_scene = {};
}

void frame()
{
    ZoneScoped;
    TracyGpuZone("frame");

    state.delta_time = stm_laptime(&state.time);

    if (state.scene_dirty)
    {
        change_scene();
        state.scene_dirty = false;
    }

    if (state.desc.scene.update)
        state.desc.scene.update();

    // Main render pass
    {
        simgui_new_frame({
            .width = sapp_width(),
            .height = sapp_height(),
            .delta_time = stm_sec(state.delta_time),
            .dpi_scale = sapp_dpi_scale(),
        });

        sg_pass const pass{
            .action = state.desc.pass_action,
            .swapchain = sglue_swapchain(),
        };
        sg_begin_pass(&pass);

        if (state.desc.scene.draw)
            state.desc.scene.draw();

        simgui_render();
        sg_end_pass();
        sg_commit();
    }

    TracyGpuCollect;
}

void cleanup()
{
    if (state.desc.scene.close)
        state.desc.scene.close();

    if (state.desc.deinit_cb)
        state.desc.deinit_cb();

    simgui_shutdown();
    sgl_shutdown();
    sg_shutdown();
}

void event(App::Event const* event)
{
    // NOTE: Touch begin events aren't properly consumed by simgui_handle_event so they're always
    // forwarded. Specifically, the begin event of the first UI touch *isn't* consumed and the begin
    // event of the first non-UI touch *is* consumed.

    if (!simgui_handle_event(event) || (event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN))
    {
        if (state.desc.scene.handle_event)
            state.desc.scene.handle_event(*event);
    }

    // Update persisted input state
    auto& input = state.input;
    switch (event->type)
    {
        case SAPP_EVENTTYPE_MOUSE_DOWN:
        {
            input.mouse_down[event->mouse_button] = true;
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP:
        {
            input.mouse_down[event->mouse_button] = false;
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_LEAVE:
        {
            for (auto& m : input.mouse_down)
                m = false;

            break;
        }
        case SAPP_EVENTTYPE_TOUCHES_BEGAN:
        case SAPP_EVENTTYPE_TOUCHES_ENDED:
        case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
        {
            input.prev_num_touches = event->num_touches;
            [[fallthrough]];
        }
        case SAPP_EVENTTYPE_TOUCHES_MOVED:
        {
            for (i32 i = 0; i < event->num_touches; ++i)
            {
                auto& p = input.prev_touch_points[i];
                p[0] = event->touches[i].pos_x;
                p[1] = event->touches[i].pos_y;
            }

            break;
        }
        default:
        {
        }
    }

    // Keep window size updated
    state.desc.window.width = event->window_width;
    state.desc.window.height = event->window_height;
}

sapp_desc sokol_app_desc()
{
    sapp_desc desc{
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = state.desc.window.width,
        .height = state.desc.window.height,
        .sample_count = 4,
        .high_dpi = true,
        .window_title = state.desc.window.title,
        .enable_clipboard = true,
        .logger{
            .func = slog_func,
        },
        .win32{
            .console_utf8 = true,
            .console_create = true,
        },
    };

    // Apply overrides
    if (state.desc.sokol_config.app)
        state.desc.sokol_config.app(desc);

    return desc;
}

} // namespace

App::Scene App::default_scene()
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

sg_pass_action App::default_pass_action()
{
    return {
        .colors{
            {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value{0.15f, 0.15f, 0.15f, 1.0f},
            },
        },
    };
}

void App::run(Desc const& desc)
{
    state.desc = desc;
    sapp_run(sokol_app_desc());
}

App::Scene const& App::scene() { return state.desc.scene; }

void App::set_scene(App::Scene const& scene)
{
    state.next_scene = scene;
    state.scene_dirty = true;
}

sg_pass_action pass_action() { return state.desc.pass_action; }

App::Input const& App::input() { return state.input; }

i32 App::framebuffer_width() { return sapp_width(); }
i32 App::framebuffer_height() { return sapp_height(); }
i32 App::window_width() { return state.desc.window.width; }
i32 App::window_height() { return state.desc.window.height; }
f32 App::aspect() { return sapp_widthf() / sapp_heightf(); }

u64 App::time() { return state.time; }
f64 App::time_s() { return stm_sec(state.time); }
f64 App::time_ms() { return stm_ms(state.time); }

u64 App::delta_time() { return state.delta_time; }
f64 App::delta_time_s() { return stm_sec(state.delta_time); }
f64 App::delta_time_ms() { return stm_ms(state.delta_time); }

} // namespace dr