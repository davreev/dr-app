#include <dr/app/app.hpp>

#include "scene.hpp"

int main(int /*argc*/, char** /*argv*/)
{
    using namespace dr;

    App::set_scene(scene());

    App::Desc desc = App::desc();
    desc.width = 1280;
    desc.height = 720;
    desc.window_title = "Example: Hello Tetra";
    desc.html5_canvas_selector = "#hello-tetra";

    App::run(desc);

    return 0;
}
