#include "scene.hpp"

#include <cmath>

#include <dr/basic_types.hpp>
#include <dr/dynamic_array.hpp>
#include <dr/linalg_reshape.hpp>
#include <dr/math.hpp>
#include <dr/math_constants.hpp>
#include <dr/math_ctors.hpp>
#include <dr/mesh_primitives.hpp>
#include <dr/span.hpp>

#include <dr/app/debug_renderer.hpp>
#include <dr/app/draw_context.hpp>
#include <dr/app/event_handlers.hpp>
#include <dr/app/gfx_utils.hpp>
#include <dr/app/orbit_camera.hpp>
#include <dr/app/shim/imgui.hpp>
#include <dr/app/shim/tracy.hpp>

namespace dr
{
namespace
{

constexpr char scene_name[]{"Example: Debug Render"};

using LineStyle = DebugRenderer::LineStyle;
using PointStyle = DebugRenderer::PointStyle;

using LineCap = DebugRenderer::LineCap;
using PointShape = DebugRenderer::PointShape;

using Pass = DebugRenderer::Pass;

constexpr f32 two_pi = 2.0 * pi<f32>;

struct
{
    DebugRenderer renderer;
    OrbitCamera camera;
    struct
    {
        DynamicArray<Vec3<f32>> points;
    } scratch;
    struct
    {
        struct
        {
            f32 spread = 0.5;
        } frame;
        struct
        {
            Vec2<f32> spacing = {0.5, 0.5};
            Vec2<i32> num_cells = {6, 6};
        } grid;
        struct
        {
            Vec3<f32> from = {-1.0, -1.0, -1.0};
            Vec3<f32> to = {1.0, 1.0, 1.0};
        } box;
        struct
        {
            f32 radius = 1.5;
            f32 inset = 0.2;
        } circle;
        struct
        {
            i32 start_face = 0;
            i32 end_face = 8;
        } dart;
        struct
        {
            f32 radius = 1.0;
            f32 height = 2.0;
        } helix;
        struct
        {
            i32 freq[2]{5, 4};
        } trefoil;
    } params;
} state{};

void open()
{
    state.camera.set_rig_now(CameraRig::make_look_at({0.0, -10.0, 4.0}, {}, {0.0, 0.0, 1.0}));
    state.camera.target.radius = 14.0f;
    state.camera.frame_target_now();
}

void close() { state = {}; }

struct Helix
{
    f32 height = 1.0f;
    f32 radius = 0.5f;
    f32 num_turns = 3.0f;

    Vec3<f32> operator()(f32 const t) const
    {
        f32 const ang = two_pi * num_turns * t;
        return {
            radius * std::cos(ang),
            radius * std::sin(ang),
            t * height,
        };
    }
};

struct Torus
{
    f32 radii[2] = {1.0, 0.5};

    Vec3<f32> operator()(f32 u, f32 v) const
    {
        u *= two_pi;
        v *= two_pi;

        auto const [r0, r1] = radii;
        f32 const cos_u = std::cos(u);

        return {
            (r0 + r1 * cos_u) * std::cos(v),
            (r0 + r1 * cos_u) * std::sin(v),
            r1 * std::sin(u),
        };
    }
};

struct Trefoil
{
    Torus torus;
    int freq[2] = {3, 2};

    Vec3<f32> operator()(f32 const t) const { return torus(freq[0] * t, freq[1] * t); }
};

void draw_scene(Mat4<f32> const& view_to_clip, Mat4<f32> const& world_to_view, f64 const time)
{
    ZoneScoped;

    auto& dbr = state.renderer;
    state.renderer.begin_frame();

    constexpr f32 loops_per_sec = 0.025;
    f32 const t_loop = fract(loops_per_sec * time);

    constexpr f32 cycles_per_sec = 0.1;
    f32 const t_cycle = 0.5 - std::cos(two_pi * cycles_per_sec * time) * 0.5; // In [0, 1]

    struct
    {
        isize x = 0;
        isize y = 0;

        Vec3<f32> next_pos()
        {
            constexpr f32 step_x = 5.0;
            constexpr f32 step_y = 5.0;

            constexpr isize count_x = 7;
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

    // Frames
    {
        Vec3<f32> const origin = layout.next_pos();
        auto& params = state.params.frame;

        dbr.draw_frame({.transform{.translation = origin}}, {.width = 5.0f});

        Mat3<f32> const l_a = Mat3<f32>::Identity();
        Vec3<f32> const t_a = {};

        Mat3<f32> const l_b = Mat3<f32>::Constant(1.0f);
        Vec3<f32> const t_b = vec<3>(1.0f);

        f32 const t = params.spread * lerp(-1.0f, 1.0f, t_cycle);

        dbr.draw_frame(
            {
                .transform{
                    .linear = l_a + t * (l_b - l_a),
                    .translation = origin + t_a + t * (t_b - t_a),
                },
            },
            {.width = 5.0f});

        dbr.draw_frame(
            {
                .transform{
                    .linear = l_a - t * (l_b - l_a),
                    .translation = origin + t_a - t * (t_b - t_a),
                },
            },
            {.width = 5.0f});
    }

    // Grids
    {
        Vec3<f32> const origin = layout.next_pos();
        auto& params = state.params.grid;

        DebugRenderer::Grid grid{
            .transform{
                .translation = origin,
            },
            .spacing = params.spacing,
            .num_cells = params.num_cells,
            .centered = true,
        };

        f32 const t = lerp(-1.0f, 1.0f, t_cycle);
        grid.transform.linear(1, 0) = t;

        dbr.draw_grid_lines(
            grid,
            {
                .color = {1.0, 1.0, 1.0, 0.4},
                .width = 2.0,
                .pass = Pass::Overlay,
            });

        grid.transform.translation.z() = 0.5f * t;
        grid.num_cells.array() -= 1;
        dbr.draw_grid_points(
            grid,
            {
                .color{1.0, 1.0, 1.0, 1.0},
                .size = 8.0,
                .shape = PointShape::Cross,
            });
    }

    // Box
    {
        Vec3<f32> const origin = layout.next_pos();
        auto& params = state.params.box;

        Mat3<f32> rotate = Mat3<f32>::Identity();
        rotate.topLeftCorner<2, 2>() = make_rotate(two_pi * cycles_per_sec * time);

        DebugRenderer::Box box{
            .transform{
                .linear = rotate,
                .translation = origin,
            },
            .from = params.from,
            .to = params.to,
        };

        dbr.draw_box(
            box,
            {
                .color = {1.0, 1.0, 1.0, 0.2},
                .width = 2.0,
                .pass = Pass::Overlay,
            });

        f32 const scale = lerp(1.0f, 0.5f, t_cycle);
        box.transform.linear *= scale;

        dbr.draw_box(
            box,
            {
                .color = {1.0, 1.0, 1.0, 1.0},
                .width = 2.0,
            });
    }

    // Circles
    {
        Vec3<f32> const origin = layout.next_pos();
        auto& params = state.params.circle;

        DebugRenderer::Circle circ{
            .transform{
                .translation = origin,
            },
            .radius = params.radius,
        };

        dbr.draw_circle(
            circ,
            {
                .color = {0.3, 0.3, 1.0, 1.0},
                .width = 4,
            });

        Mat2<f32> const rot = make_rotate(two_pi * cycles_per_sec * time);

        Mat3<f32> rot_yz = Mat3<f32>::Identity();
        rot_yz({1, 2}, {1, 2}) = rot;
        circ.transform.linear *= rot_yz;
        circ.radius -= params.radius * params.inset;

        dbr.draw_circle(
            circ,
            {
                .color = {0.3, 1.0, 0.3, 1.0},
                .width = 4,
            });

        Mat3<f32> rot_zx = Mat3<f32>::Identity();
        rot_zx({2, 0}, {2, 0}) = rot;
        circ.transform.linear *= rot_zx;
        circ.radius -= params.radius * params.inset;

        dbr.draw_circle(
            circ,
            {
                .color = {1.0, 0.3, 0.3, 1.0},
                .width = 4,
            });
    }

    // Darts
    {
        Vec3<f32> const origin = layout.next_pos();
        auto& params = state.params.dart;

        auto const& mesh = MeshPrimitives::Tri::icosahedron();
        constexpr f32 scale = 1.5f;

        constexpr auto mix = [](Vec3<f32> const& a, Vec3<f32> const& b, f32 const t) -> Vec3<f32> {
            return a + t * (b - a);
        };

        auto draw_dart = [&](Vec3<f32> const& p0, Vec3<f32> const& p1, Vec3<f32> const& p2) {
            constexpr f32 inv3 = 1.0 / 3.0;
            Vec3<f32> const& cen = inv3 * (p0 + p1 + p2);

            constexpr f32 t = 0.9;
            Vec3<f32> const d0 = (p0 - cen) * t;
            Vec3<f32> const d1 = (p1 - cen) * t;
            Vec3<f32> const d2 = (p2 - cen) * t;

            f32 const u = lerp(1.0f, 0.5f, t_cycle);
            Vec3<f32> const d01 = mix(d0, d1, u);
            Vec3<f32> const d12 = mix(d1, d2, u);

            Vec3<f32> const pad = 0.1 * (d12 - d01);
            Vec3<f32> const a = cen + d01 + pad;
            Vec3<f32> const b = cen + d12 - pad;
            dbr.draw_dart(
                {
                    .start = origin + scale * a,
                    .end = origin + scale * b,
                    .up = cen - b,
                },
                {
                    .color = {1.0, 1.0, 1.0, 1.0},
                    .width = 2.0,
                });
        };

        isize const num_faces = mesh.face_vertices.size();
        isize const num_drawn_faces = params.end_face - params.start_face;
        for (isize i = 0; i < num_drawn_faces; ++i)
        {
            int const f = mod(params.start_face + i, num_faces);
            auto& f_v = mesh.face_vertices[f];
            Vec3<f32> const p[] = {
                as_vec(mesh.vertex_positions[f_v[0]]),
                as_vec(mesh.vertex_positions[f_v[1]]),
                as_vec(mesh.vertex_positions[f_v[2]]),
            };
            draw_dart(p[0], p[1], p[2]);
            draw_dart(p[1], p[2], p[0]);
            draw_dart(p[2], p[0], p[1]);
        }

        isize const num_edges = mesh.edge_vertices.size();
        for (isize e = 0; e < num_edges; ++e)
        {
            auto& v = mesh.edge_vertices[e];
            Vec3<f32> const p0 = as_vec(mesh.vertex_positions[v[0]]);
            Vec3<f32> const p1 = as_vec(mesh.vertex_positions[v[1]]);
            dbr.draw_line(
                origin + scale * p0,
                origin + scale * p1,
                {
                    .color = {1.0, 1.0, 1.0, 0.1},
                    .width = 1.0,
                    .pass = Pass::Overlay,
                });
        }

        isize const num_verts = mesh.vertex_positions.size();
        for (isize v = 0; v < num_verts; ++v)
        {
            Vec3<f32> const p = as_vec(mesh.vertex_positions[v]);
            dbr.draw_point(
                origin + scale * p,
                {
                    .color = {1.0, 1.0, 1.0, 1.0},
                    .size = 3.0,
                });
        }
    }

    // Helix
    {
        Vec3<f32> const origin = layout.next_pos();
        auto& params = state.params.helix;

        Helix const helix{
            .height = params.height,
            .radius = params.radius,
            .num_turns = lerp(1.0f, 4.0f, t_cycle),
        };

        DebugRenderer::Curve curve{
            .transform{
                .translation = origin,
            },
            .eval = &helix,
            .domain = {-0.5, 0.5},
            .num_segments = 128,
        };

        dbr.draw_curve(
            curve,
            {
                .color = {1.0, 1.0, 1.0, 0.1},
                .width = 16.0,
                .pass = Pass::Opaque,
            });

        dbr.draw_curve(
            curve,
            {
                .color = {1.0, 1.0, 1.0, 0.2},
                .width = 8.0,
                .pass = Pass::Opaque,
            });

        curve.num_segments = 16;

        dbr.draw_curve_points(
            curve,
            {
                .color = {1.0, 1.0, 1.0, 1.0},
                .size = 8.0,
                .pass = Pass::Opaque,
            });
    }

    // Trefoil knot
    {
        Vec3<f32> const origin = layout.next_pos();
        auto& params = state.params.trefoil;

        Trefoil const trefoil{.freq = {params.freq[0], params.freq[1]}};

        DebugRenderer::Curve curve{
            .transform{
                .translation = origin,
            },
            .eval = &trefoil,
            .domain = {t_loop, t_loop + 1.0f},
            .segment_trim = {0.0, 0.5},
            .num_segments = 128,
        };

        dbr.draw_curve(
            curve,
            {
                .color = {1.0, 1.0, 1.0, 1.0},
                .width = 3.0,
                .pass = Pass::Opaque,
            });
    }

    dbr.submit({
        .world_to_clip = view_to_clip * world_to_view,
        .viewport{
            .size = {sapp_widthf(), sapp_heightf()},
            .dpi_scale = sapp_dpi_scale(),
        },
    });
}

void draw_ui()
{
    ZoneScoped;

    App::begin_ui();

    ImGui::SetNextWindowPos({20.0f, 20.0f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints({300.0f, 0.0f}, {sapp_widthf(), sapp_heightf()});
    constexpr auto window_flags = ImGuiWindowFlags_NoResize;

    ImGui::Begin(scene_name, nullptr, window_flags);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Params"))
        {
            ImGui::Text("FPS: %.2f", App::profiler().fps());

            ImGui::SeparatorText("Frame");
            {
                auto& p = state.params.frame;
                ImGui::SliderFloat("Spread", &p.spread, 0.0f, 1.0f);
            }

            ImGui::SeparatorText("Grid");
            {
                auto& p = state.params.grid;
                ImGui::SliderFloat2("Spacing", p.spacing.data(), 0.0f, 1.0f);
                ImGui::SliderInt2("Num cells", p.num_cells.data(), 1, 16);
            }

            ImGui::SeparatorText("Box");
            {
                auto& p = state.params.box;
                ImGui::SliderFloat3("From", p.from.data(), -1.0, 0.0);
                ImGui::SliderFloat3("To", p.to.data(), 0.0, 1.0);
            }

            ImGui::SeparatorText("Circle");
            {
                auto& p = state.params.circle;
                ImGui::SliderFloat("Radius##1", &p.radius, 0.0, 2.0);
                ImGui::SliderFloat("Inset", &p.inset, 0.0, 1.0);
            }

            ImGui::SeparatorText("Dart");
            {
                auto& p = state.params.dart;
                ImGui::SliderInt("Start face", &p.start_face, 0, 16);
                ImGui::SliderInt("End face", &p.end_face, 0, 16);
            }

            ImGui::SeparatorText("Helix");
            {
                auto& p = state.params.helix;
                ImGui::SliderFloat("Radius##2", &p.radius, 0.0, 2.0);
                ImGui::SliderFloat("Height", &p.height, 0.0, 5.0);
            }

            ImGui::SeparatorText("Trefoil");
            {
                auto& p = state.params.trefoil;
                ImGui::SliderInt2("Freq", p.freq, 1, 10);
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("About"))
        {
            ImGui::Text("Version %u.%u.%u", 0, 1, 0);
            ImGui::TextLinkOpenURL(
                "Source",
                "https://github.com/davreev/dr-app/tree/master/example/debug-render");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    App::end_ui();
}

void draw()
{
    App::begin_swapchain_pass();

    auto const& cam = state.camera;
    Mat4<f32> const view_to_clip = cam.make_view_to_clip(App::aspect());
    Mat4<f32> const world_to_view = cam.make_world_to_view();

    draw_scene(view_to_clip, world_to_view, App::time_s());
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
