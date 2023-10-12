#include <dr/app/event_handlers.hpp>

#include <cmath>

#include <sokol_app.h>

#include <dr/app/camera.hpp>

namespace dr
{

bool is_mouse_over(sapp_event const* const event)
{
    return event->mouse_x >= 0 && event->mouse_x < event->window_width && event->mouse_y >= 0
        && event->mouse_y < event->window_height;
}

f32 screen_to_view(f32 const fov, f32 const size)
{
    return std::tan(fov * 0.5f) / (size * 0.5f);
}

void camera_handle_mouse_event(
    sapp_event const& event,
    f32 const camera_distance,
    f32 const screen_to_view,
    Orbit* const orbit,
    Zoom* const zoom,
    Pan* const pan,
    bool mouse_down[3])
{
    switch (event.type)
    {
        case SAPP_EVENTTYPE_MOUSE_DOWN:
        {
            if (event.mouse_button == SAPP_MOUSEBUTTON_LEFT)
                mouse_down[0] = true;

            if (event.mouse_button == SAPP_MOUSEBUTTON_RIGHT)
                mouse_down[1] = true;

            if (event.mouse_button == SAPP_MOUSEBUTTON_MIDDLE)
                mouse_down[2] = true;

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP:
        {
            if (event.mouse_button == SAPP_MOUSEBUTTON_LEFT)
                mouse_down[0] = false;

            if (event.mouse_button == SAPP_MOUSEBUTTON_RIGHT)
                mouse_down[1] = false;

            if (event.mouse_button == SAPP_MOUSEBUTTON_MIDDLE)
                mouse_down[2] = false;

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_LEAVE:
        {
            mouse_down[0] = false;
            mouse_down[1] = false;
            mouse_down[2] = false;
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_MOVE:
        {
            if (mouse_down[0] && orbit != nullptr)
            {
                Vec2<f32> const d{event.mouse_dx, event.mouse_dy};
                orbit->handle_drag(d * screen_to_view);
            }

            if (mouse_down[1] && pan != nullptr)
            {
                Vec2<f32> const d{event.mouse_dx, event.mouse_dy};
                pan->handle_drag(d * (camera_distance * screen_to_view));
            }

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
        {
            constexpr f32 scroll_scale = 0.1f;

            if (zoom != nullptr)
            {
                f32 const d = scroll_scale * sign(event.scroll_y);
                zoom->handle_scroll(d * camera_distance);
            }

            break;
        }
        default:
        {
            // ...
        }
    }
}

void camera_handle_touch_event(
    sapp_event const& event,
    f32 const camera_distance,
    f32 const screen_to_view,
    Orbit* const orbit,
    Zoom* const zoom,
    Pan* const /*pan*/,
    Vec2<f32> last_touch_points[2],
    i8& last_num_touches)
{
    switch (event.type)
    {
        case SAPP_EVENTTYPE_TOUCHES_BEGAN:
        {
            last_num_touches = 0;
            break;
        }
        case SAPP_EVENTTYPE_TOUCHES_MOVED:
        {
            auto const& touches = event.touches;
            Vec2<f32> const p0{touches[0].pos_x, touches[0].pos_y};
            Vec2<f32> const p1{touches[1].pos_x, touches[1].pos_y};

            if (event.num_touches != last_num_touches)
            {
                // Start new gesture
                last_num_touches = event.num_touches;
            }
            else
            {
                // Process current gesture
                switch (event.num_touches)
                {
                    case 1:
                    {
                        if (orbit != nullptr)
                        {
                            Vec2<f32> const d = p0 - last_touch_points[0];
                            orbit->handle_drag(d * screen_to_view);
                        }
                        break;
                    }
                    case 2:
                    {
                        // TODO: Handle as pan if deltas for each point are nearly parallel
                        if (zoom != nullptr)
                        {
                            f32 const d0 = (last_touch_points[0] - last_touch_points[1]).norm();
                            f32 const d1 = (p0 - p1).norm();
                            zoom->handle_scroll((d1 - d0) * (camera_distance * screen_to_view));
                        }
                        break;
                    }
                }
            }

            last_touch_points[0] = p0;
            last_touch_points[1] = p1;
            break;
        }
        default:
        {
            // ...
        }
    }
}

} // namespace dr