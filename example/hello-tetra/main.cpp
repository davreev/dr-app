#include <dr/app/app.hpp>

#include "scene.hpp"

dr::App::Desc DR_APP_MAIN(int /*argc*/, char** /*argv*/)
{
    using namespace dr;

    App::set_scene(scene());

    App::Desc desc = App::desc();
    {
        desc.width = 1280;
        desc.height = 720;
        desc.window_title = "Example: Hello Tetra";
#if __EMSCRIPTEN__
        desc.html5_canvas_name = "hello-tetra";
#endif
    }
    return desc;
}
