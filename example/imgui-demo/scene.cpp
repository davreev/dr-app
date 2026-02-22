#include "scene.hpp"

#include <dr/basic_types.hpp>

#include <dr/app/shim/imgui.hpp>

#include <implot.h>
#include <implot3d.h>

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
        .name = "ImGui Demo",
        .draw = draw,
    };
}

} // namespace dr