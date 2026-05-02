#include "scene.hpp"

#include <implot.h>
#include <implot3d.h>

#include <dr/basic_types.hpp>

#include <dr/app/shim/imgui.hpp>

namespace dr
{
namespace
{

void draw()
{
    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
    ImPlot3D::ShowDemoWindow();
}

} // namespace

App::Scene scene()
{
    return {
        .name = "Example: ImGui Demo",
        .draw = draw,
    };
}

} // namespace dr