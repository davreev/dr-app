#include "scene.hpp"

#include <sokol_time.h>

#include <dr/basic_types.hpp>
#include <dr/dynamic_array.hpp>
#include <dr/linalg_reshape.hpp>
#include <dr/math.hpp>
#include <dr/memory.hpp>
#include <dr/span.hpp>

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

constexpr char scene_name[]{"Example: Draw Streams"};
constexpr NdcType ndc = NdcType::NdcType_OpenGl;

struct RenderMesh
{
    struct Vertex
    {
        Vec3<f32> position;
        Vec3<f32> color;
    };
    using Index = i32;

    GfxBuffer vertices;
    GfxBuffer indices;
    i32 index_count;
};

struct
{
    struct
    {
        GfxShader shader;
        GfxPipeline pipeline;
    } gfx;
    RenderMesh mesh;
    struct
    {
        DynamicArray<RenderMesh::Vertex> vertices;
        DynamicArray<RenderMesh::Index> indices;
    } scratch;
    DrawContext draw_ctx;
    OrbitCamera camera;
} state{};

struct Disc
{
    static constexpr i32 num_segments = 32;

    static void update(
        DynamicArray<RenderMesh::Vertex>& verts,
        DynamicArray<RenderMesh::Index>& indices,
        f64 t)
    {
        constexpr f64 loop_dur = 4.0f;
        t = fract(t / loop_dur);

        {
            constexpr f32 radius = 1.5;
            constexpr f32 two_pi = 2.0 * pi<f32>;

            verts.clear();
            verts.reserve(num_segments + 1);

            Vec3<f32> const base_col{0.4, 0.8, 1.0};
            f32 const sin_t = std::sin(two_pi * t);
            f32 const pos_z = 2.0f * sin_t;
            f32 const col_bump = 0.8f * sin_t;

            verts.push_back({
                .position = {0.0f, 0.0f, pos_z},
                .color = (base_col.array() + col_bump).min(1.0).max(0.0),
            });

            for (i32 i = 0; i < num_segments; ++i)
            {
                f32 const ang = i * (two_pi / num_segments);
                verts.push_back({
                    .position = {radius * std::cos(ang), radius * std::sin(ang), 0.0f},
                    .color = base_col,
                });
            }
        }

        {
            f32 const tri_t = 1.0 - 2.0 * abs(t - 0.5);
            i32 const num_tris = num_segments - i32(num_segments * tri_t);

            indices.clear();
            indices.reserve(num_tris * 3);

            for (i32 i = 0; i < num_tris; ++i)
            {
                indices.push_back(0);
                indices.push_back(1 + i % num_segments);
                indices.push_back(1 + (i + 1) % num_segments);
            }
        }
    }

    static void init(
        DynamicArray<RenderMesh::Vertex>& verts,
        DynamicArray<RenderMesh::Index>& indices)
    {
        update(verts, indices, 0.0);
    };
};

using Shape = Disc;

void init_gfx()
{
    auto& gfx = state.gfx;
    gfx.shader = GfxShader::make({
        .vertex_func{.source = vertex_shader_src},
        .fragment_func{.source = fragment_shader_src},
        .uniform_blocks{
            // [0] Pass
            {
                .stage = SG_SHADERSTAGE_VERTEX,
                .size = sizeof(f32[16]),
                .glsl_uniforms{{SG_UNIFORMTYPE_MAT4, 1, "u_view_to_clip"}},
            },
            {}, // [1] Material (unused)
            {}, // [2] Geometry (unused)
            // [3] Object
            {
                .stage = SG_SHADERSTAGE_VERTEX,
                .size = sizeof(f32[16]),
                .glsl_uniforms{{SG_UNIFORMTYPE_MAT4, 1, "u_local_to_view"}},
            },
        },
    });
    gfx.pipeline = GfxPipeline::make({
        .shader = gfx.shader,
        .layout{
            .attrs{
                {.buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3},
                {.buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3},
            },
        },
        .depth{
            .compare = SG_COMPAREFUNC_LESS,
            .write_enabled = true,
        },
        .index_type = SG_INDEXTYPE_UINT32,
        .cull_mode = SG_CULLMODE_NONE,
        .face_winding = SG_FACEWINDING_CCW,
    });
}

void init_mesh()
{
    auto& [verts, indices] = state.scratch;
    Shape::init(verts, indices);

    auto vertex_data = as<u8>(as_span(verts));
    auto index_data = as<u8>(as_span(indices));

    state.mesh = {
        .vertices = GfxBuffer::make({
            .usage = {.vertex_buffer = true, .immutable = true},
            .data = {vertex_data.data(), usize(vertex_data.size())},
        }),
        .indices = GfxBuffer::make({
            .usage = {.index_buffer = true, .immutable = true},
            .data = {index_data.data(), usize(index_data.size())},
        }),
        .index_count = i32(indices.size()),
    };
}

void open()
{
    init_gfx();
    init_mesh();

    state.camera.set_rig_now(CameraRig::make_look_at({0.0, -1.0, 1.0}, {}, {0.0, 0.0, 1.0}));
    state.camera.target.radius = 10.0f;
    state.camera.frame_target_now();
}

void close() { state = {}; }

i32 draw_ctx_push_object(Mat4<f32> const& local_to_view)
{
    struct
    {
        f32 local_to_view[16];
    } u;
    as_mat<4, 4>(u.local_to_view) = local_to_view;
    return state.draw_ctx.uniform_stream.push(as_bytes(u));
}

void draw_ctx_submit_pass(Mat4<f32> const& view_to_clip)
{
    struct
    {
        f32 view_to_clip[16];
    } u;
    as_mat<4, 4>(u.view_to_clip) = view_to_clip;
    state.draw_ctx.submit_draw_cmds({.uniform_data = as_bytes(u)});
}

Vec3<f32> pass_position(i32 const i)
{
    constexpr f32 step_x = 6.0f;
    return {-9.0f + step_x * f32(i), 0.0f, 0.0f};
}

void draw_scene(Mat4<f32> const& view_to_clip, Mat4<f32> const& world_to_view)
{
    ZoneScoped;

    auto& draw_ctx = state.draw_ctx;
    auto& mesh = state.mesh;
    auto& scratch = state.scratch;

    // Draw context streams accumulate across passes and reset here
    draw_ctx.begin_frame();

    struct GeometrySrc
    {
        GfxBuffer::Handle vertices;
        VertexStream* vertex_stream;
        GfxBuffer::Handle indices;
        IndexStream* index_stream;
    };

    auto draw_pass = [&](GeometrySrc const& geom, Vec3<f32> const& pos) {
        DrawCommand cmd{
            .pipeline = state.gfx.pipeline,
            .geometry = &geom,
            .set_bindings =
                [](DrawCommand const& cmd, sg_bindings& bindings) {
                    auto const* geom = static_cast<GeometrySrc const*>(cmd.geometry);
                    bindings.vertex_buffers[0] = geom->vertex_stream //
                        ? geom->vertex_stream->device_buffer()
                        : geom->vertices;
                    bindings.index_buffer = geom->index_stream //
                        ? geom->index_stream->device_buffer()
                        : geom->indices;
                },
            .uniform_slices{.object = draw_ctx_push_object(world_to_view * make_translate(pos))},
        };

        if (geom.vertex_stream)
        {
            cmd.buffer_offsets.vertex[0] = draw_ctx.vertex_stream.push_once<0>(
                &mesh.vertices,
                as<u8>(as_span(scratch.vertices)));
        }

        if (geom.index_stream)
        {
            cmd.buffer_offsets.index = draw_ctx.index_stream.push_once(
                &mesh.indices,
                as<u8>(as_span(scratch.indices)));
            cmd.args.num_elements = i32(scratch.indices.size());
        }
        else
        {
            cmd.args.num_elements = mesh.index_count;
        }

        draw_ctx.draw_cmds.push_back(cmd);
        draw_ctx_submit_pass(view_to_clip);
    };

    /*
        NOTE(dr): Each pass emits a single draw command using either a static buffer or a stream for
        vertex and index dara. For streamed data, byte offsets are stored on the draw command so
        that the stream buffers can be bound correctly during submission.
    */

    draw_pass(
        {
            .vertices = mesh.vertices,
            .indices = mesh.indices,
        },
        pass_position(0));
    draw_pass(
        {
            .vertex_stream = &draw_ctx.vertex_stream,
            .indices = mesh.indices,
        },
        pass_position(1));
    draw_pass(
        {
            .vertices = mesh.vertices,
            .index_stream = &draw_ctx.index_stream,
        },
        pass_position(2));
    draw_pass(
        {
            .vertex_stream = &draw_ctx.vertex_stream,
            .index_stream = &draw_ctx.index_stream,
        },
        pass_position(3));
}

void draw_ui()
{
    ZoneScoped;

    ImGui::SetNextWindowPos({20.0f, 20.0f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints({280.0f, 0.0f}, {sapp_widthf(), sapp_heightf()});
    constexpr auto window_flags = ImGuiWindowFlags_NoResize;

    ImGui::Begin(scene_name, nullptr, window_flags);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Passes"))
        {
            ImGui::Text("FPS: %.2f", App::profiler().fps());
            ImGui::SeparatorText("Key");
            ImGui::BulletText("A: static vertices, static indices");
            ImGui::BulletText("B: streamed vertices, static indices");
            ImGui::BulletText("C: static vertices, streamed indices");
            ImGui::BulletText("D: streamed vertices, streamed indices");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("About"))
        {
            ImGui::Text("Version %u.%u.%u", 0, 1, 0);
            ImGui::TextLinkOpenURL(
                "Source",
                "https://github.com/davreev/dr-app/tree/master/example/draw-streams");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void draw_label(char const* const text, Vec3<f32> const& world_pos, Mat4<f32> const& world_to_clip)
{
    Vec4<f32> const p_clip = world_to_clip * world_pos.homogeneous();

    // Check if behind the camera
    if (p_clip.w() <= 0.0f)
        return;

    Vec2<f32> const display = ImGui::GetIO().DisplaySize;
    Vec2<f32> const p_screen{
        (p_clip.x() / p_clip.w() * 0.5f + 0.5f) * display.x(),
        (1.0f - (p_clip.y() / p_clip.w() * 0.5f + 0.5f))
            * display.y(), // NDC is y-up, screen is y-down
    };

    Vec2<f32> const size = ImGui::CalcTextSize(text);
    Vec2<f32> const min = p_screen - 0.5f * size;
    ImGui::GetForegroundDrawList()->AddText(min, IM_COL32(255, 255, 255, 255), text);
}

void draw_labels(Mat4<f32> const& world_to_clip)
{
    constexpr i32 pass_count = 4;
    static constexpr char const* labels[pass_count]{"A", "B", "C", "D"};
    Vec3<f32> const offset{0.0f, 2.5f, 0.0f};
    for (i32 i = 0; i < pass_count; ++i)
        draw_label(labels[i], pass_position(i) + offset, world_to_clip);
}

void draw()
{
    App::begin_swapchain_pass();

    auto const& cam = state.camera;
    Mat4<f32> const view_to_clip = cam.make_view_to_clip<ndc>(App::aspect());
    Mat4<f32> const world_to_view = cam.make_world_to_view();

    draw_scene(view_to_clip, world_to_view);
    draw_ui();
    draw_labels(view_to_clip * world_to_view);
    sgl_draw();

    App::end_swapchain_pass();
}

void update_mesh(f64 const t)
{
    auto& [verts, indices] = state.scratch;
    Shape::update(verts, indices, t);
}

void update()
{
    state.camera.update(App::delta_time_s());
    update_mesh(App::time_s());
    draw();
}

void handle_event(App::Event const& event)
{
    camera_handle_mouse_event(event, state.camera);
    camera_handle_touch_event(event, state.camera);

    if (event.type == SAPP_EVENTTYPE_KEY_DOWN && event.key_code == SAPP_KEYCODE_F)
    {
        if (event.modifiers & SAPP_MODIFIER_SHIFT)
            state.camera.frame_target_now();
        else
            state.camera.frame_target();
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
