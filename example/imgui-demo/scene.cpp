#include "scene.hpp"

#include <implot.h>
#include <implot3d.h>
#include <sokol_gfx.h>

#include <dr/app/shim/imgui.hpp>

namespace dr
{
namespace
{

void draw_ui()
{
    App::begin_ui();

    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
    ImPlot3D::ShowDemoWindow();

    App::end_ui();
}

void update()
{
    sg_begin_pass(App::swapchain_pass());
    draw_ui();
    sg_end_pass();
}

} // namespace

App::Scene scene()
{
    return {
        .name = "Example: ImGui Demo",
        .update = update,
    };
}

} // namespace dr
