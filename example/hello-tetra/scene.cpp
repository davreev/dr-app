#include "scene.hpp"

#include <dr/basic_types.hpp>
#include <dr/bitwise.hpp>
#include <dr/math.hpp>

#include <dr/app/debug_draw.hpp>
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

constexpr char scene_name[]{"Hello Tetra"};

// clang-format off

struct {
    struct {
        GfxPipeline pipeline;
        GfxShader shader;
        GfxBuffer vertex_buffer;
        GfxBuffer index_buffer;
    } gfx;

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
    GfxShader::Desc desc{};
    desc.vertex_func.source = vertex_src;
    desc.fragment_func.source = fragment_src;
    desc.uniform_blocks[0].stage = SG_SHADERSTAGE_VERTEX;
    desc.uniform_blocks[0].size = sizeof(f32[16]);
    desc.uniform_blocks[0].glsl_uniforms[0] = {SG_UNIFORMTYPE_MAT4, 1, "u_local_to_clip"};
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

void init_gfx()
{
    auto& gfx = state.gfx;
    gfx.shader = GfxShader::make(shader_desc(vertex_shader_src, fragment_shader_src));
    gfx.pipeline = GfxPipeline::make(pipeline_desc(gfx.shader));
    gfx.vertex_buffer = GfxBuffer::make(
        buffer_desc(SG_RANGE(mesh_vertices), SG_BUFFERTYPE_VERTEXBUFFER));
    gfx.index_buffer = GfxBuffer::make(
        buffer_desc(SG_RANGE(mesh_indices), SG_BUFFERTYPE_INDEXBUFFER));
}

void open(void* /*context*/)
{
    init_gfx();

    state.camera.target.radius = 2.0f;
    state.camera.frame_target_now();
}

void close(void* /*context*/) { state.gfx = {}; }

void update(void* /*context*/) { state.camera.update(App::delta_time_s()); }

void draw_mesh(Mat4<f32> const& local_to_view, Mat4<f32> const& view_to_clip)
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
        struct
        {
            f32 local_to_clip[16];
        } uniforms;

        as_mat<4, 4>(uniforms.local_to_clip) = view_to_clip * local_to_view;
        sg_apply_uniforms(0, {&uniforms, sizeof(uniforms)});
    }

    sg_draw(0, 12, 1);
}

void debug_draw(
    Mat4<f32> const& local_to_view,
    Mat4<f32> const& world_to_view,
    Mat4<f32> const& view_to_clip)
{
    sgl_defaults();

    sgl_matrix_mode_projection();
    sgl_load_matrix(view_to_clip.data());

    debug_draw_axes(world_to_view, 0.25f);
    debug_draw_unit_cube_edges(local_to_view);

    sgl_draw();
}

void draw_ui()
{
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
}

void draw(void* /*context*/)
{
    auto const& cam = state.camera;
    Mat4<f32> const world_to_view = cam.make_world_to_view();
    Mat4<f32> const view_to_clip = cam.make_view_to_clip<NdcType_OpenGl>(App::aspect());

    Mat4<f32> const local_to_world = make_scale_translate(vec<3>(2.0f), vec<3>(-1.0f));
    Mat4<f32> const local_to_view = world_to_view * local_to_world;

    draw_mesh(local_to_view, view_to_clip);
    debug_draw(local_to_view, world_to_view, view_to_clip);
    draw_ui();
}

void handle_event(void* /*context*/, App::Event const& event)
{
    camera_handle_mouse_event(event, state.camera);
    camera_handle_touch_event(event, state.camera);

    switch (event.type)
    {
        case SAPP_EVENTTYPE_KEY_DOWN:
        {
            switch (event.key_code)
            {
                case SAPP_KEYCODE_F:
                {
                    state.camera.frame_target();
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

App::Scene scene() { return {scene_name, open, close, update, draw, handle_event, nullptr}; }

} // namespace dr