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
        desc.window_title = "Example: Hello DR";
#if __EMSCRIPTEN__
        desc.html5_canvas_name = "hello-dr";
#endif
    }
    return desc;
}
