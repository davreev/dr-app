#include "scene.hpp"

#include <sokol_time.h>

#include <dr/basic_types.hpp>
#include <dr/dynamic_array.hpp>
#include <dr/linalg_reshape.hpp>
#include <dr/math.hpp>
#include <dr/memory.hpp>
#include <dr/span.hpp>

#include <dr/app/draw_streams.hpp>
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
    Index index_count;
};

struct
{
    struct
    {
        GfxShader shader;
        GfxPipeline pipeline;
    } gfx;
    RenderMesh mesh;
    DynamicArray<RenderMesh::Vertex> vertices;
    DynamicArray<RenderMesh::Index> indices;
    VertexStream vertex_stream;
    IndexStream index_stream;
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
        constexpr f64 cycles_per_sec = 0.25f;
        t = fract(cycles_per_sec * t);

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
    Shape::init(state.vertices, state.indices);

    auto vertex_data = as<u8>(as_span(state.vertices));
    auto index_data = as<u8>(as_span(state.indices));

    state.mesh = {
        .vertices = GfxBuffer::make({
            .usage = {.vertex_buffer = true, .immutable = true},
            .data = {vertex_data.data(), usize(vertex_data.size())},
        }),
        .indices = GfxBuffer::make({
            .usage = {.index_buffer = true, .immutable = true},
            .data = {index_data.data(), usize(index_data.size())},
        }),
        .index_count = i32(state.indices.size()),
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

void draw_label(char const* const text, Vec3<f32> const& world_pos, Mat4<f32> const& world_to_clip)
{
    Vec4<f32> const p_clip = world_to_clip * world_pos.homogeneous();

    // Check if behind the camera
    if (p_clip.w() <= 0.0f)
        return;

    Vec2<f32> const display = ImGui::GetIO().DisplaySize;
    Vec2<f32> const p_screen{
        (p_clip.x() / p_clip.w() * 0.5f + 0.5f) * display.x(),
        (1.0f - (p_clip.y() / p_clip.w() * 0.5f + 0.5f)) * display.y(), // Assumes NDC is y-up
    };

    Vec2<f32> const size = ImGui::CalcTextSize(text);
    Vec2<f32> const min = p_screen - 0.5f * size;
    ImGui::GetForegroundDrawList()->AddText(min, IM_COL32(255, 255, 255, 255), text);
}

void draw_scene()
{
    ZoneScoped;

    auto const& cam = state.camera;
    Mat4<f32> const view_to_clip = cam.make_view_to_clip(App::aspect());
    Mat4<f32> const world_to_view = cam.make_world_to_view();
    Mat4<f32> const world_to_clip = view_to_clip * world_to_view;

    struct
    {
        isize x = 0;
        isize y = 0;

        Vec3<f32> next_pos()
        {
            constexpr f32 step_x = 5.0;
            constexpr f32 step_y = 5.0;

            constexpr isize count_x = 4;
            constexpr f32 offset_x = -0.5f * step_x * (count_x - 1);

            Vec3<f32> const p = {step_x * x + offset_x, step_y * y, 0.0};
            if (++x >= count_x)
            {
                ++y;
                x = 0;
            }
            return p;
        };
    } layout;

    auto& mesh = state.mesh;
    auto& vs = state.vertex_stream;
    auto& is = state.index_stream;

    sg_apply_pipeline(state.gfx.pipeline);

    // Set pass uniforms
    struct
    {
        f32 view_to_clip[16]{};
    } pass_uniforms;
    as_mat<4, 4>(pass_uniforms.view_to_clip) = view_to_clip;
    sg_apply_uniforms(0, {&pass_uniforms, sizeof(pass_uniforms)});

    auto draw_shape = [&](char const* label, bool stream_verts, bool stream_indices) {
        Vec3<f32> const pos = layout.next_pos();
        Mat4<f32> const local_to_view = world_to_view * make_translate(pos);

        // Set object uniforms
        struct
        {
            f32 local_to_view[16]{};
        } obj_uniforms;
        as_mat<4, 4>(obj_uniforms.local_to_view) = local_to_view;
        sg_apply_uniforms(3, {&obj_uniforms, sizeof(obj_uniforms)});

        // Bind buffers
        sg_apply_bindings(
            sg_bindings{
                .vertex_buffers{
                    stream_verts ? vs.device_buffer() : mesh.vertices.handle(),
                },
                .index_buffer = stream_indices ? is.device_buffer() : mesh.indices.handle(),
            });

        // Draw mesh
        i32 const num_indices = stream_indices ? i32(state.indices.size()) : mesh.index_count;
        sg_draw(0, num_indices, 1);

        // Draw label
        Vec3<f32> const label_offset{0.0f, 2.5f, 0.0f};
        draw_label(label, pos + label_offset, world_to_clip);
    };

    draw_shape("A", false, false);
    draw_shape("B", true, false);
    draw_shape("C", false, true);
    draw_shape("D", true, true);
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

void draw()
{
    App::begin_swapchain_pass();
    App::begin_ui();

    draw_scene();
    draw_ui();

    App::end_ui();
    App::end_swapchain_pass();
}

void update_mesh(f64 const t)
{
    Shape::update(state.vertices, state.indices, t);
    state.vertex_stream.push(as<u8>(as_span(state.vertices)));
    state.index_stream.push(as<u8>(as_span(state.indices)));
}

void update()
{
    auto& vs = state.vertex_stream;
    auto& is = state.index_stream;

    // Streams are typically reset once per frame
    state.vertex_stream.reset();
    state.index_stream.reset();

    state.camera.update(App::delta_time_s());
    update_mesh(App::time_s());

    // Push additional data to streams here
    // ...

    vs.transfer();
    is.transfer();

    // Issue draw calls using streamed data
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
