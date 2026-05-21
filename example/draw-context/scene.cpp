#include "scene.hpp"

#include <sokol_time.h>

#include <dr/basic_types.hpp>
#include <dr/linalg_reshape.hpp>
#include <dr/math.hpp>
#include <dr/math_ctors.hpp>
#include <dr/memory.hpp>

#include <dr/app/draw_context.hpp>
#include <dr/app/event_handlers.hpp>
#include <dr/app/gfx_resource.hpp>
#include <dr/app/gfx_utils.hpp>
#include <dr/app/orbit_camera.hpp>
#include <dr/app/shim/imgui.hpp>
#include <dr/app/shim/tracy.hpp>

#include "shader_src.hpp"

namespace dr
{
namespace
{

constexpr char scene_name[]{"Example: Draw Context"};
constexpr NdcType ndc = NdcType::NdcType_OpenGl;

struct Fps
{
    Fps() = default;

    void start() { prev_time_ = stm_now(); }

    f64 current() const { return current_; }

    void update()
    {
        constexpr usize interval = 1000000000;
        ++frame_count_;

        usize const time = stm_now();
        usize const elapsed = time - prev_time_;

        if (elapsed > interval)
        {
            current_ = frame_count_ / stm_sec(elapsed);
            frame_count_ = 0;
            prev_time_ = time;
        }
    }

  private:
    usize prev_time_{};
    usize frame_count_{};
    f64 current_{};
};

// clang-format off

struct {
    struct {
        GfxPipeline pipeline;
        GfxShader shader;
        struct {
            GfxBuffer vertex_buffer;
            GfxBuffer index_buffer;
        } mesh;
    } gfx;

    DynamicArray<Vec3<f32>> instances;
    DrawContext draw_ctx;
    OrbitCamera camera;
    Fps fps;
} state{};

// clang-format on

Span<f32 const[6]> mesh_vertices()
{
    // Format: {x, y, z, r, g, b}
    static constexpr f32 verts[][6]{
        {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f},
        {1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.5f},
        {0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.5f},
        {1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f},
    };

    return verts;
}

Span<i16 const[3]> mesh_indices()
{
    static constexpr i16 indices[][3]{
        {0, 3, 2},
        {1, 2, 3},
        {2, 1, 0},
        {3, 0, 1},
    };

    return indices;
}

void init_instances()
{
    auto& instances = state.instances;

    constexpr i32 grid_size = 512;
    constexpr f32 grid_spacing = 2.0f;
    constexpr f32 half_extent = grid_size * grid_spacing * 0.5f;

    instances.resize(grid_size * grid_size);
    i32 idx = 0;

    for (i32 y = 0; y < grid_size; ++y)
    {
        for (i32 x = 0; x < grid_size; ++x)
            instances[idx++] = vec(
                x * grid_spacing - half_extent,
                y * grid_spacing - half_extent,
                0.0f);
    }
}

void update_instances(f64 const time_s)
{
    ZoneScoped;

    auto& instances = state.instances;

    constexpr f32 offset_s = 0.06f;
    constexpr f32 cycles_per_s = 0.2f;
    constexpr f32 s_to_rad = 2.0f * pi<f32> * cycles_per_s;

#pragma omp parallel for schedule(static)
    for (isize i = 0; i < size(instances); ++i)
    {
        auto& p = instances[i];
        p.z() = std::sin(s_to_rad * (time_s + i * offset_s));
    }
}

GfxShader::Desc shader_desc(char const* vertex_src, char const* fragment_src)
{
    return {
        .vertex_func{.source = vertex_src},
        .fragment_func{.source = fragment_src},
        .uniform_blocks{
            // [0] Pass
            {
                .stage = SG_SHADERSTAGE_VERTEX,
                .size = sizeof(f32[16]),
                .glsl_uniforms{
                    {SG_UNIFORMTYPE_MAT4, 1, "u_view_to_clip"},
                },
            },
            // [1] Material (unused)
            {},
            // [2] Geometry (unused)
            {},
            // [3] Object
            {
                .stage = SG_SHADERSTAGE_VERTEX,
                .size = sizeof(f32[16]),
                .glsl_uniforms{
                    {SG_UNIFORMTYPE_MAT4, 1, "u_local_to_view"},
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
            .buffers{
                {.step_func = SG_VERTEXSTEP_PER_VERTEX},
                {.step_func = SG_VERTEXSTEP_PER_INSTANCE},
            },
            .attrs{
                {.buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3},
                {.buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3},
                {.buffer_index = 1, .format = SG_VERTEXFORMAT_FLOAT3},
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

    auto const vertex_data = as<u8>(mesh_vertices());
    gfx.mesh.vertex_buffer = GfxBuffer::make({
        .usage = {.vertex_buffer = true, .immutable = true},
        .data = {vertex_data.data(), usize(vertex_data.size())},
    });

    auto const index_data = as<u8>(mesh_indices());
    gfx.mesh.index_buffer = GfxBuffer::make({
        .usage = {.index_buffer = true, .immutable = true},
        .data = {index_data.data(), usize(index_data.size())},
    });
}

void open()
{
    init_gfx();
    init_instances();

    state.camera.target.radius = 20.0f;
    state.camera.frame_target_now();

    state.fps.start();
}

void close()
{
    state.gfx = {};
    state.draw_ctx = {};
}

void update()
{
    update_instances(App::time_s());
    state.camera.update(App::delta_time_s());
    state.fps.update();
}

void draw_scene(Mat4<f32> const& view_to_clip, Mat4<f32> const& world_to_view)
{
    ZoneScoped;

    auto& draw_ctx = state.draw_ctx;

    // NOTE(dr): Using immutable buffers for static mesh data and the draw context's vertex stream
    // for dynamic instance data

    struct Geometry
    {
        GfxBuffer::Handle vertex;
        GfxBuffer::Handle index;
        VertexStream* stream;
    };

    Geometry const geom{
        .vertex = state.gfx.mesh.vertex_buffer,
        .index = state.gfx.mesh.index_buffer,
        .stream = &draw_ctx.vertex_stream,
    };

    // Create draw commands
    {
        struct
        {
            f32 local_to_view[16];
        } obj_uniforms;
        as_mat<4, 4>(obj_uniforms.local_to_view) = world_to_view;

        draw_ctx.draw_cmds.push_back({
            .pipeline = state.gfx.pipeline,
            .geometry = &geom,
            .set_bindings =
                [](DrawCommand const& cmd, sg_bindings& bindings) {
                    auto geom = static_cast<Geometry const*>(cmd.geometry);
                    bindings.vertex_buffers[0] = geom->vertex;
                    bindings.vertex_buffers[1] = geom->stream->device_buffer();
                    bindings.vertex_buffer_offsets[1] = cmd.vertex_offsets[0];
                    bindings.index_buffer = geom->index;
                },
            .vertex_offsets{
                draw_ctx.vertex_stream.push_once<0>(&geom, as<u8>(as_span(state.instances))),
            },
            .uniform_slices{
                .object = draw_ctx.uniform_stream.push(as_bytes(obj_uniforms)),
            },
            .num_elements = 3 * i32(mesh_indices().size()),
            .num_instances = i32(state.instances.size()),
        });
    }

    // Submit draw commands
    {
        ZoneScopedN("submit draw commands");
        TracyGpuZone("submit draw commands");

        struct
        {
            f32 view_to_clip[16];
        } pass_uniforms;
        as_mat<4, 4>(pass_uniforms.view_to_clip) = view_to_clip;

        draw_ctx.submit_draw_cmds({
            .uniform_data = as_bytes(pass_uniforms),
        });
    }
}

void draw_ui()
{
    ZoneScoped;

    ImGui::SetNextWindowPos({20.0f, 20.0f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints({220.0f, 0.0f}, {sapp_widthf(), sapp_heightf()});
    constexpr auto window_flags = ImGuiWindowFlags_NoResize;

    ImGui::Begin(scene_name, nullptr, window_flags);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Settings"))
        {
            ImGui::SeparatorText("Scene");
            {
                int const num_instances = size(state.instances);
                ImGui::Text("Instance count: %d", num_instances);
                ImGui::Text("FPS: %.3f", state.fps.current());
            }

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
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("About"))
        {
            ImGui::Text("Version %u.%u.%u", 0, 1, 0);
            ImGui::TextLinkOpenURL(
                "Source",
                "https://github.com/davreev/dr-app/tree/master/example/draw-context");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void draw()
{
    auto const& cam = state.camera;
    draw_scene(cam.make_view_to_clip<ndc>(App::aspect()), cam.make_world_to_view());
    draw_ui();
}

void handle_event(App::Event const& event)
{
    camera_handle_mouse_event(event, state.camera);
    camera_handle_touch_event(event, state.camera);

    static CameraRig const alt_rig = CameraRig::make_look_at(
        {20.0f, 20.0f, 20.0f},
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
        .draw = draw,
        .handle_event = handle_event,
    };
}

} // namespace dr
