#include <dr/app/app.hpp>

#include "scene.hpp"

int main(int /*argc*/, char** /*argv*/)
{
    using namespace dr;

    App::run({
        .scene = scene(),
        .sokol_config{
            .app = [](sapp_desc& desc) { desc.html5.canvas_selector = "#debug-render"; },
        },
    });

    return 0;
}
