#include "scene.hpp"

#include <dr/basic_types.hpp>
#include <dr/bitwise.hpp>
#include <dr/linalg_reshape.hpp>
#include <dr/math.hpp>

#include <dr/app/debug_renderer.hpp>
#include <dr/app/event_handlers.hpp>
#include <dr/app/gfx_resource.hpp>
#include <dr/app/gfx_utils.hpp>
#include <dr/app/orbit_camera.hpp>
#include <dr/app/shim/imgui.hpp>

#include "shader_src.hpp"

namespace dr
{
namespace
{

constexpr char scene_name[]{"Example: Hello Tetra"};

// clang-format off

struct {
    struct {
        GfxPipeline pipeline;
        GfxShader shader;
        GfxBuffer vertex_buffer;
        GfxBuffer index_buffer;
    } gfx;

    DebugRenderer renderer{};
    OrbitCamera camera{};
} state{};

// clang-format on

// Format: {x, y, z, r, g, b}
constexpr f32 mesh_vertices[][6]{
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.5},
    {1.0, 1.0, 0.0, 1.0, 0.0, 0.5},
    {0.0, 1.0, 1.0, 0.0, 1.0, 0.5},
    {1.0, 0.0, 1.0, 1.0, 1.0, 0.5},
};

constexpr i16 const mesh_indices[][3]{
    {0, 3, 2},
    {1, 2, 3},
    {2, 1, 0},
    {3, 0, 1},
};

GfxShader::Desc shader_desc(char const* vertex_src, char const* fragment_src)
{
    return {
        .vertex_func{.source = vertex_src},
        .fragment_func{.source = fragment_src},
        .uniform_blocks{
            {
                .stage = SG_SHADERSTAGE_VERTEX,
                .size = sizeof(f32[16]),
                .glsl_uniforms{
                    {SG_UNIFORMTYPE_MAT4, 1, "u_local_to_clip"},
                },
            },
        },
    };
}

GfxPipeline::Desc pipeline_desc(GfxShader const& shader)
{
    return {
        .shader = shader,
        .layout{
            .attrs{
                {.format = SG_VERTEXFORMAT_FLOAT3},
                {.format = SG_VERTEXFORMAT_FLOAT3},
            },
        },
        .depth{
            .compare = SG_COMPAREFUNC_LESS,
            .write_enabled = true,
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_NONE,
        .face_winding = SG_FACEWINDING_CCW,
    };
}

void init_gfx()
{
    auto& gfx = state.gfx;
    gfx.shader = GfxShader::make(shader_desc(vertex_shader_src, fragment_shader_src));
    gfx.pipeline = GfxPipeline::make(pipeline_desc(gfx.shader));
    gfx.vertex_buffer = GfxBuffer::make({
        .usage{.vertex_buffer = true, .immutable = true},
        .data = SG_RANGE(mesh_vertices),
    });
    gfx.index_buffer = GfxBuffer::make({
        .usage{.index_buffer = true, .immutable = true},
        .data = SG_RANGE(mesh_indices),
    });
}

void open()
{
    init_gfx();

    state.camera.target.radius = 2.0f;
    state.camera.frame_target_now();
}

void close() { state.gfx = {}; }

void draw_mesh(Mat4<f32> const& local_to_view, Mat4<f32> const& view_to_clip)
{
    sg_apply_pipeline(state.gfx.pipeline);

    // Apply bindings
    sg_apply_bindings({
        .vertex_buffers{
            state.gfx.vertex_buffer,
        },
        .index_buffer = state.gfx.index_buffer,
    });

    // Apply uniforms
    {
        struct
        {
            f32 local_to_clip[16];
        } uniforms;

        as_mat<4, 4>(uniforms.local_to_clip) = view_to_clip * local_to_view;
        sg_apply_uniforms(0, {&uniforms, sizeof(uniforms)});
    }

    sg_draw(0, 12, 1);
}

Affine3<f32> to_affine(Mat4<f32> const& m)
{
    return {
        .linear = m.topLeftCorner<3, 3>(),
        .translation = m.topRightCorner<3, 1>(),
    };
}

void draw_debug(Mat4<f32> const& local_to_world, Mat4<f32> const& world_to_clip)
{
    auto& dbr = state.renderer;
    dbr.begin_frame();
    dbr.draw_frame({.scale = 0.2}, {.width = 3.0});
    dbr.draw_box(
        {
            .transform = to_affine(local_to_world),
        },
        {
            .color = {1.0, 1.0, 1.0, 0.1},
            .width = 2.0,
            .pass = DebugRenderer::Pass::Overlay,
        });
    dbr.submit({
        .world_to_clip = world_to_clip,
        .viewport{
            .size = {sapp_widthf(), sapp_heightf()},
            .dpi_scale = sapp_dpi_scale(),
        },
    });
}

void draw_ui()
{
    App::begin_ui();

    ImGui::SetNextWindowPos({20.0f, 20.0f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints({200.0f, 0.0f}, {sapp_widthf(), sapp_heightf()});
    constexpr auto window_flags = ImGuiWindowFlags_NoResize;

    ImGui::Begin(scene_name, nullptr, window_flags);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Settings"))
        {
            ImGui::SeparatorText("Camera");
            {
                int proj = state.camera.projection;
                ImGui::Text("Projection");
                ImGui::RadioButton("Perspective", &proj, 0);
                ImGui::SameLine();
                ImGui::RadioButton("Orthographic", &proj, 1);
                state.camera.projection = OrbitCamera::Projection(proj);

                ImGui::BeginDisabled(proj == 1);
                ImGui::SliderFloat("FOV", &state.camera.frustum.fov_y, 0.0f, pi<f32>);
                ImGui::EndDisabled();

                ImGui::Spacing();
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("About"))
        {
            ImGui::Text("Version %u.%u.%u", 0, 2, 0);
            ImGui::TextLinkOpenURL(
                "Source",
                "https://github.com/davreev/dr-app/tree/master/example/hello-tetra");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    App::end_ui();
}

void draw()
{
    auto const& cam = state.camera;
    Mat4<f32> const world_to_view = cam.make_world_to_view();
    Mat4<f32> const view_to_clip = cam.make_view_to_clip(App::aspect());

    Mat4<f32> const local_to_world = make_scale_translate(vec<3>(2.0f), vec<3>(-1.0f));
    Mat4<f32> const local_to_view = world_to_view * local_to_world;

    App::begin_swapchain_pass();
    draw_mesh(local_to_view, view_to_clip);
    App::end_swapchain_pass();

    App::begin_swapchain_pass({
        .colors{
            {.load_action = SG_LOADACTION_LOAD},
        },
    });
    draw_debug(local_to_world, view_to_clip * world_to_view);
    draw_ui();

    App::end_swapchain_pass();
}

void update()
{
    state.camera.update(App::delta_time_s());
    draw();
}

void handle_event(App::Event const& event)
{
    camera_handle_mouse_event(event, state.camera);
    camera_handle_touch_event(event, state.camera);

    static CameraRig const alt_rig = CameraRig::make_look_at(
        {5.0f, 5.0f, 5.0f},
        {},
        {0.0f, 0.0f, 1.0f});

    switch (event.type)
    {
        case SAPP_EVENTTYPE_KEY_DOWN:
        {
            switch (event.key_code)
            {
                case SAPP_KEYCODE_F:
                {
                    if (event.modifiers & SAPP_MODIFIER_SHIFT)
                        state.camera.frame_target_now();
                    else
                        state.camera.frame_target();

                    break;
                }
                case SAPP_KEYCODE_G:
                {
                    if (event.modifiers & SAPP_MODIFIER_SHIFT)
                        state.camera.set_rig_now(alt_rig);
                    else
                        state.camera.set_rig(alt_rig);

                    break;
                }
                case SAPP_KEYCODE_1:
                {
                    state.camera.set_rig(CameraRig::make_front({}, 5.0f));
                    break;
                }
                case SAPP_KEYCODE_2:
                {
                    state.camera.set_rig(CameraRig::make_back({}, 5.0f));
                    break;
                }
                case SAPP_KEYCODE_3:
                {
                    state.camera.set_rig(CameraRig::make_right({}, 5.0f));
                    break;
                }
                case SAPP_KEYCODE_4:
                {
                    state.camera.set_rig(CameraRig::make_left({}, 5.0f));
                    break;
                }
                case SAPP_KEYCODE_5:
                {
                    state.camera.set_rig(CameraRig::make_top({}, 5.0f));
                    break;
                }
                case SAPP_KEYCODE_6:
                {
                    state.camera.set_rig(CameraRig::make_bottom({}, 5.0f));
                    break;
                }
                default:
                {
                    // ...
                }
            }
            break;
        }
        default:
        {
            // ...
        }
    }
}

} // namespace

App::Scene scene()
{
    return {
        .name = scene_name,
        .open = open,
        .close = close,
        .update = update,
        .handle_event = handle_event,
    };
}

} // namespace dr