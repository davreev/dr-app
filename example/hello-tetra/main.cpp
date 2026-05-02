#include <dr/app/app.hpp>

#include "scene.hpp"

int main(int /*argc*/, char** /*argv*/)
{
    using namespace dr;

    App::run({
        .scene = scene(),
        .window{
            .title = "Example: Hello Tetra",
        },
        .sokol_config{
            .app = [](sapp_desc& desc) { desc.html5.canvas_selector = "#hello-tetra"; },
        },
    });

    return 0;
}
