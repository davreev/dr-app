#include "scene.hpp"

#include <dr/basic_types.hpp>
#include <dr/bitwise.hpp>
#include <dr/math.hpp>

#include <dr/app/camera.hpp>
#include <dr/app/debug_draw.hpp>
#include <dr/app/event_handlers.hpp>
#include <dr/app/gfx_resource.hpp>
#include <dr/app/gfx_utils.hpp>
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

    struct {
        bool mouse_down[3];
        Vec2<f32> last_touch_points[2];
        i8 last_num_touches;
    } input;

    struct {
        struct {
            f32 fov_y{deg_to_rad(60.0f)};
            f32 clip_near{0.01f};
            f32 clip_far{100.0f};
        } frustum;

        struct {
            EasedOrbit orbit{{0.15f * pi<f32>, 0.35f * pi<f32>}};
            EasedZoom zoom{{4.0f, 1.0f, 0.1f}};
            EasedPan pan{Pan{}};
        } controls;

        Camera camera{make_camera(controls.orbit.current, controls.zoom.current)};
    } view;

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

void init_gfx_resources()
{
    auto& gfx = state.gfx;
    gfx.shader = GfxShader::make(shader_desc(vertex_shader_src, fragment_shader_src));
    gfx.pipeline = GfxPipeline::make(pipeline_desc(gfx.shader));
    gfx.vertex_buffer = GfxBuffer::make(
        buffer_desc(SG_RANGE(mesh_vertices), SG_BUFFERTYPE_VERTEXBUFFER));
    gfx.index_buffer = GfxBuffer::make(
        buffer_desc(SG_RANGE(mesh_indices), SG_BUFFERTYPE_INDEXBUFFER));
}

void open(void* /*context*/) { init_gfx_resources(); }

void close(void* /*context*/) { state.gfx = {}; }

void update(void* /*context*/)
{
    f32 const t = saturate(5.0 * App::delta_time_s());
    auto& view = state.view;

    view.controls.orbit.update(t);
    view.controls.orbit.apply(view.camera);

    view.controls.zoom.update(t);
    view.controls.zoom.apply(view.camera);

    view.controls.pan.update(t);
    view.controls.pan.apply(view.camera);
}

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
    constexpr auto window_flags = ImGuiWindowFlags_NoResize;

    ImGui::Begin(scene_name, nullptr, window_flags);
    ImGui::PushItemWidth(200.0f);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("About"))
        {
            ImGui::Text("Version %u.%u.%u", 0, 1, 0);
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
    auto const& view = state.view;

    Mat4<f32> const local_to_world = make_scale_translate(vec<3>(2.0f), vec<3>(-1.0f));
    Mat4<f32> const world_to_view = view.camera.transform().inverse_to_matrix();

    Mat4<f32> const local_to_view = world_to_view * local_to_world;
    Mat4<f32> const view_to_clip = make_perspective<NdcType_OpenGl>(
        view.frustum.fov_y,
        App::aspect(),
        view.frustum.clip_near,
        view.frustum.clip_far);

    draw_mesh(local_to_view, view_to_clip);
    debug_draw(local_to_view, world_to_view, view_to_clip);
    draw_ui();
}

void handle_event(void* /*context*/, App::Event const& event)
{
    auto& view = state.view;
    f32 const screen_to_view = dr::screen_to_view(view.frustum.fov_y, sapp_heightf());

    camera_handle_mouse_event(
        event,
        view.controls.zoom.target,
        &view.controls.orbit.target,
        &view.controls.pan.target,
        screen_to_view,
        state.input.mouse_down);

    camera_handle_touch_event(
        event,
        view.controls.zoom.target,
        &view.controls.orbit.target,
        &view.controls.pan.target,
        screen_to_view,
        state.input.last_touch_points,
        state.input.last_num_touches);

    constexpr auto center_camera = []() {
        view.controls.zoom.target.distance = 4.0f;
        view.controls.pan.target.offset = {};
    };

    switch (event.type)
    {
        case SAPP_EVENTTYPE_KEY_DOWN:
        {
            switch (event.key_code)
            {
                case SAPP_KEYCODE_F:
                {
                    center_camera();
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