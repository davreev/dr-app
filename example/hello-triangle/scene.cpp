#include "scene.hpp"

#include <dr/basic_types.hpp>
#include <dr/math.hpp>

#include <dr/app/camera.hpp>
#include <dr/app/camera_utils.hpp>
#include <dr/app/event_handlers.hpp>
#include <dr/app/gfx.hpp>
#include <dr/app/gfx_utils.hpp>
#include <dr/app/shim/imgui.hpp>

namespace dr
{
namespace
{

// clang-format off

struct {
    char const* name = "Hello Triangle";
    char const* author = "David Reeves";
    char const* year = "2023";
    struct {
        u16 major{0};
        u16 minor{1};
        u16 patch{0};
    } version;
} const scene_info;

struct {
    struct {
        GfxPipeline pipeline;
        GfxShader shader;
        GfxBuffer vertex_buffer;
        GfxBuffer index_buffer;
    } gfx;

    struct {
        bool mouse_down[3];
    } input;

    struct {
        f32 fov_y{deg_to_rad(60.0f)};
        f32 clip_near{0.01f};
        f32 clip_far{100.0f};
    } view;

    struct {
        Orbit orbit{0.0f, 0.5f * pi<f32>};
        Zoom zoom{2.0f};
        Camera target{make_camera(orbit, zoom)};
        Camera current{target};
    } camera;
} state;

// clang-format on

char const vertex_shader_src[] = R"(#version 330 core

uniform mat4 u_local_to_clip;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;

out vec3 v_color;

void main()
{
    gl_Position = u_local_to_clip * vec4(a_position, 1.0);
    v_color = a_color;
}

)";

char const fragment_shader_src[] = R"(#version 330 core

in vec3 v_color;

out vec4 f_color;

void main()
{
    f_color = vec4(v_color, 1.0);
}

)";

// clang-format off

f32 const mesh_vertices[]{
    // x, y, z, r, g, b
    -1.0, 0.0, -1.0, 1.0, 0.0, 0.5, 
    1.0, 0.0, -1.0, 0.0, 1.0, 0.5,
    0.0, 0.0, 1.0, 0.0, 0.0, 0.5,
};

// clang-format on

i16 const mesh_indices[]{0, 1, 2};

GfxShader::Desc shader_desc(char const* vertex_src, char const* fragment_src)
{
    GfxShader::Desc desc{};
    desc.vs.source = vertex_src;
    desc.vs.uniform_blocks[0].uniforms[0] = {"u_local_to_clip", SG_UNIFORMTYPE_MAT4, 1};
    desc.vs.uniform_blocks[0].size = 16 * sizeof(f32);
    desc.fs.source = fragment_src;
    return desc;
}

GfxPipeline::Desc pipeline_desc(GfxShader const& shader)
{
    GfxPipeline::Desc desc{};
    desc.shader = shader;
    desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
    desc.depth.compare = SG_COMPAREFUNC_LESS;
    desc.depth.write_enabled = true;
    desc.index_type = SG_INDEXTYPE_UINT16;
    desc.face_winding = SG_FACEWINDING_CCW;
    desc.cull_mode = SG_CULLMODE_NONE;
    return desc;
}

GfxBuffer::Desc buffer_desc(sg_range const& data, sg_buffer_type const type)
{
    GfxBuffer::Desc desc{};
    desc.data = data;
    desc.type = type;
    return desc;
}

void init_gfx_resources()
{
    auto& gfx = state.gfx;
    gfx.shader.init(shader_desc(vertex_shader_src, fragment_shader_src));
    gfx.pipeline.init(pipeline_desc(gfx.shader));
    gfx.vertex_buffer.init(buffer_desc(SG_RANGE(mesh_vertices), SG_BUFFERTYPE_VERTEXBUFFER));
    gfx.index_buffer.init(buffer_desc(SG_RANGE(mesh_indices), SG_BUFFERTYPE_INDEXBUFFER));
}

void open() { init_gfx_resources(); }

void close() { state.gfx = {}; }

void update()
{
    auto& cam = state.camera;
    cam.current.transition_to(cam.target, saturate(5.0 * app_delta_time_s()));
}

void draw_mesh()
{
    sg_apply_pipeline(state.gfx.pipeline);

    // Apply bindings
    {
        sg_bindings bindings{};
        bindings.vertex_buffers[0] = state.gfx.vertex_buffer;
        bindings.index_buffer = state.gfx.index_buffer;
        sg_apply_bindings(bindings);
    }

    // Apply uniforms
    {
        Mat4<f32> const world_to_view = state.camera.current.transform().inverse_to_matrix();
        Mat4<f32> const view_to_clip = make_perspective<NdcType_OpenGl>(
            state.view.fov_y,
            app_aspect(),
            state.view.clip_near,
            state.view.clip_far);

        struct
        {
            f32 local_to_clip[16];
        } vs_uniforms;

        as_mat<4, 4>(vs_uniforms.local_to_clip) = view_to_clip * world_to_view;
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, {&vs_uniforms, sizeof(vs_uniforms)});
    }

    sg_draw(0, size(mesh_indices), 1);
}

void draw_ui()
{
    ImGui::SetNextWindowPos({20.0f, 20.0f}, ImGuiCond_FirstUseEver);
    constexpr int window_flags = ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::Begin(scene_info.name, nullptr, window_flags);
    ImGui::PushItemWidth(200.0f);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("About"))
        {
            ImGui::Text(
                "Version %u.%u.%u",
                scene_info.version.major,
                scene_info.version.minor,
                scene_info.version.patch);

            ImGui::Text("Copyright %s %s", scene_info.author, scene_info.year);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void draw()
{
    draw_mesh();
    draw_ui();
}

void handle_event(sapp_event const* const event)
{
    f32 const move_scale = screen_to_view_scale(state.view.fov_y, sapp_heightf());

    camera_handle_mouse_event(
        state.camera.target,
        &state.camera.orbit,
        &state.camera.zoom,
        nullptr,
        state.input.mouse_down,
        move_scale,
        *event);
}

} // namespace

App::Scene const* scene()
{
    static App::Scene const scene{
        scene_info.name,
        open,
        close,
        update,
        draw,
        handle_event,
    };

    return &scene;
}

} // namespace dr