#include "scene.hpp"

#include <implot.h>
#include <implot3d.h>

#include <dr/basic_types.hpp>

#include <dr/app/shim/imgui.hpp>

namespace dr
{
namespace
{

void update()
{
    App::begin_swapchain_pass();
    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
    ImPlot3D::ShowDemoWindow();
    App::end_swapchain_pass();
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