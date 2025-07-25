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

f32 screen_to_view_scale(f32 const fov, f32 const size)
{
    return std::tan(fov * 0.5f) / (size * 0.5f);
}

f32 screen_to_view_scale(f32 const fov_y) { return screen_to_view_scale(fov_y, sapp_heightf()); }

Mat3<f32> make_screen_to_view(f32 const fov_y, bool const flip_y)
{
    f32 const w = sapp_widthf();
    f32 const h = sapp_heightf();
    f32 const sx = screen_to_view_scale(fov_y, h);
    f32 const sy = flip_y ? -sx : sx;

    Mat3<f32> m;
    m(0, 0) = sx;
    m(1, 1) = sy;
    m.col(2) << sx * w * -0.5f, sy * h * -0.5f, 1.0f;
    return m;
}

Mat3<f32> make_view_to_screen(f32 const fov_y, bool const flip_y)
{
    f32 const w = sapp_widthf();
    f32 const h = sapp_heightf();
    f32 const s = 1.0f / screen_to_view_scale(fov_y, h);

    Mat3<f32> m;
    m(0, 0) = s;
    m(1, 1) = flip_y ? -s : s;
    m.col(2) << w * 0.5f, h * 0.5f, 1.0f;
    return m;
}

void camera_handle_mouse_event(
    App::Event const& event,
    Zoom& zoom,
    Orbit* const orbit,
    Pan* const pan,
    f32 const drag_scale,
    f32 const scroll_scale,
    bool mouse_down[3])
{
    f32 const cam_offset = zoom.distance;
    f32 const screen_norm = 1.0 / sapp_heightf();

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
                orbit->handle_input(d * (screen_norm * drag_scale));
            }

            if (mouse_down[1] && pan)
            {
                Vec2<f32> const d{event.mouse_dx, event.mouse_dy};
                pan->handle_input(d * (cam_offset * screen_norm * drag_scale));
            }

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
        {
            f32 const d = sign(event.scroll_y);
            zoom.handle_input(d * cam_offset * scroll_scale);
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
    f32 const drag_scale,
    Vec2<f32> prev_touch_points[2],
    i8& prev_num_touches)
{
    f32 const cam_offset = zoom.distance;
    f32 const screen_norm = 1.0 / sapp_heightf();

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
                            orbit->handle_input(d * (screen_norm * drag_scale));
                        }
                        break;
                    }
                    case 2:
                    {
                        // Handle drag pan
                        if (pan)
                        {
                            Vec2<f32> const d0 = screen_norm * (p0 - prev_touch_points[0]);
                            Vec2<f32> const d1 = screen_norm * (p1 - prev_touch_points[1]);

                            constexpr f32 parallel_tol = 0.01f;
                            if (near_equal(d0, d1, parallel_tol))
                                pan->handle_input((d0 + d1) * (0.5f * cam_offset * drag_scale));
                        }

                        // Handle pinch zoom
                        {
                            f32 const d0 = (prev_touch_points[0] - prev_touch_points[1]).norm();
                            f32 const d1 = (p0 - p1).norm();
                            zoom.handle_input((d1 - d0) * (cam_offset * screen_norm * drag_scale));
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