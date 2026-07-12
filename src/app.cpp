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
    App::Profiler profiler{};
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

    state.profiler.start();
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
    state.profiler.update();

    if (state.scene_dirty)
    {
        change_scene();
        state.scene_dirty = false;
    }

    if (state.desc.scene.update)
        state.desc.scene.update();

    sg_commit();

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
    // NOTE(dr): simgui_handle_event returns true if ImGui wants to capture mouse *or* keyboard,
    // making it too coarse to drive event forwarding on its own. Instead we always feed the event
    // to ImGui and then decide per-event-type whether to forward it to the scene, consulting the
    // appropriate WantCapture flag for that event category.
    simgui_handle_event(event);

    // NOTE(dr): Touch events are routed into ImGui as mouse input (see simgui_add_touch_*), so
    // they're gated by WantCaptureMouse alongside the mouse events.
    constexpr auto is_mouse_event = [](sapp_event_type const t) {
        switch (t)
        {
            case SAPP_EVENTTYPE_MOUSE_DOWN:
            case SAPP_EVENTTYPE_MOUSE_UP:
            case SAPP_EVENTTYPE_MOUSE_SCROLL:
            case SAPP_EVENTTYPE_MOUSE_MOVE:
            case SAPP_EVENTTYPE_MOUSE_ENTER:
            case SAPP_EVENTTYPE_MOUSE_LEAVE:
            case SAPP_EVENTTYPE_TOUCHES_BEGAN:
            case SAPP_EVENTTYPE_TOUCHES_MOVED:
            case SAPP_EVENTTYPE_TOUCHES_ENDED:
            case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
                return true;
            default:
                return false;
        }
    };

    constexpr auto is_key_event = [](sapp_event_type const t) {
        switch (t)
        {
            case SAPP_EVENTTYPE_KEY_DOWN:
            case SAPP_EVENTTYPE_KEY_UP:
            case SAPP_EVENTTYPE_CHAR:
                return true;
            default:
                return false;
        }
    };

    if (state.desc.scene.handle_event)
    {
        auto const& io = ImGui::GetIO();
        bool const fwd_event = !(io.WantCaptureMouse && is_mouse_event(event->type))
            && !(io.WantCaptureKeyboard && is_key_event(event->type));

        if (fwd_event)
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
    auto& scene = state.desc.scene;
    auto& window = state.desc.window;

    sapp_desc desc{
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = window.width,
        .height = window.height,
        .sample_count = 4,
        .high_dpi = true,
        .window_title = window.title ? window.title : scene.name,
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

sg_pass_action default_pass_action()
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

} // namespace

void App::begin_swapchain_pass(sg_pass_action const& action)
{
    sg_pass const pass{
        .action = action,
        .swapchain = sglue_swapchain(),
    };
    sg_begin_pass(&pass);

    simgui_new_frame({
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = stm_sec(state.delta_time),
        .dpi_scale = sapp_dpi_scale(),
    });
}

void App::begin_swapchain_pass() { begin_swapchain_pass(default_pass_action()); }

void App::end_swapchain_pass()
{
    simgui_render();
    sg_end_pass();
}

void App::run(Desc const& desc)
{
    state.desc = desc;
    sapp_run(sokol_app_desc());
}

App::Scene const& App::scene() { return state.desc.scene; }

App::Scene App::default_scene()
{
    return {
        .name = "Default Scene",
        .update =
            []() {
                App::begin_swapchain_pass();
                ImGui::BeginTooltip();
                ImGui::Text("This is the default scene. Nothing to see here.");
                ImGui::EndTooltip();
                App::end_swapchain_pass();
            },
    };
}

void App::set_scene(App::Scene const& scene)
{
    state.next_scene = scene;
    state.scene_dirty = true;
}

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

App::Profiler& App::profiler() { return state.profiler; }

/*
    App::Profiler impl
*/

void App::Profiler::start() { start_time = stm_now(); }

void App::Profiler::update()
{
    if (++frame_count == frame_interval)
    {
        u64 const curr_time = stm_now();
        elapsed_time = curr_time - start_time;
        start_time = curr_time;
        frame_count = 0;
    }
}

u64 App::Profiler::frame_duration() const { return elapsed_time / frame_interval; }
f64 App::Profiler::frame_duration_s() const { return stm_sec(elapsed_time) / frame_interval; }
f64 App::Profiler::frame_duration_ms() const { return stm_ms(elapsed_time) / frame_interval; }
f64 App::Profiler::fps() const { return frame_interval / stm_sec(elapsed_time); }

} // namespace dr