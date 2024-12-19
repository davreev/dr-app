#include <dr/app/event_handlers.hpp>

#include <cmath>

#include <dr/math.hpp>
#include <dr/app/app.hpp>
#include <dr/app/camera.hpp>

namespace dr
{

bool is_mouse_over(App::Event const& event)
{
    return event.mouse_x >= 0 && event.mouse_x < event.window_width && event.mouse_y >= 0
        && event.mouse_y < event.window_height;
}

f32 screen_to_view(f32 const fov, f32 const size) { return std::tan(fov * 0.5f) / (size * 0.5f); }

void camera_handle_mouse_event(
    App::Event const& event,
    Zoom& zoom,
    Orbit* const orbit,
    Pan* const pan,
    f32 const screen_to_view,
    bool mouse_down[3])
{
    f32 const cam_offset = zoom.distance;

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
            if (mouse_down[0] && orbit)
            {
                Vec2<f32> const d{event.mouse_dx, event.mouse_dy};
                orbit->handle_drag(d * screen_to_view);
            }

            if (mouse_down[1] && pan)
            {
                Vec2<f32> const d{event.mouse_dx, event.mouse_dy};
                pan->handle_drag(d * (cam_offset * screen_to_view));
            }

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
        {
            constexpr f32 scroll_scale = 0.1f; // TODO(dr): Expose as parameter
            f32 const d = scroll_scale * sign(event.scroll_y);
            zoom.handle_scroll(d * cam_offset);
            break;
        }
        default:
        {
            // ...
        }
    }
}

void camera_handle_touch_event(
    App::Event const& event,
    Zoom& zoom,
    Orbit* const orbit,
    Pan* const pan,
    f32 const screen_to_view,
    Vec2<f32> prev_touch_points[2],
    i8& prev_num_touches)
{
    f32 const cam_offset = zoom.distance;

    switch (event.type)
    {
        case SAPP_EVENTTYPE_TOUCHES_BEGAN:
        {
            prev_num_touches = 0;
            break;
        }
        case SAPP_EVENTTYPE_TOUCHES_MOVED:
        {
            auto const& touches = event.touches;
            Vec2<f32> const p0{touches[0].pos_x, touches[0].pos_y};
            Vec2<f32> const p1{touches[1].pos_x, touches[1].pos_y};

            if (event.num_touches != prev_num_touches)
            {
                // Start new gesture
                prev_num_touches = event.num_touches;
            }
            else
            {
                // Process current gesture
                switch (event.num_touches)
                {
                    case 1:
                    {
                        if (orbit)
                        {
                            Vec2<f32> const d = p0 - prev_touch_points[0];
                            orbit->handle_drag(d * screen_to_view);
                        }
                        break;
                    }
                    case 2:
                    {
                        // Handle drag pan
                        if(pan)
                        {
                            Vec2<f32> const d0 = screen_to_view * (p0 - prev_touch_points[0]);
                            Vec2<f32> const d1 = screen_to_view * (p1 - prev_touch_points[1]);

                            constexpr f32 abs_tol = 1.0e-2;
                            if(near_equal(d0, d1, abs_tol))
                                pan->handle_drag((d0 + d1) * (0.5f * cam_offset));
                        }

                        // Handle pinch zoom
                        {
                            f32 const d0 = (prev_touch_points[0] - prev_touch_points[1]).norm();
                            f32 const d1 = (p0 - p1).norm();
                            zoom.handle_scroll((d1 - d0) * (cam_offset * screen_to_view));
                        }
                        break;
                    }
                }
            }

            prev_touch_points[0] = p0;
            prev_touch_points[1] = p1;
            break;
        }
        default:
        {
            // ...
        }
    }
}

} // namespace dr