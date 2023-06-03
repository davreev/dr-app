#include <dr/app/utils.hpp>

#include <sokol_app.h>

#include <dr/math.hpp>

namespace dr
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// I/O

bool read_text_file(char const* const path, std::string& buffer)
{
    std::ifstream in{path, std::ios::in};

    if (in)
    {
        using Iter = std::istreambuf_iterator<char>;
        buffer.assign(Iter{in}, {});
        return true;
    }
    else
    {
        return false;
    }
}

bool read_binary_file(char const* const path, std::vector<u8>& buffer)
{
    std::ifstream in{path, std::ios::in | std::ios::binary};

    if (in)
    {
        using Iter = std::istreambuf_iterator<char>;
        buffer.assign(Iter{in}, {});
        return true;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Input

bool is_mouse_over(sapp_event const* const event)
{
    return event->mouse_x >= 0 && event->mouse_x < event->window_width &&
        event->mouse_y >= 0 && event->mouse_y < event->window_height;
}

f32 screen_to_view_scale(f32 const fov, f32 const size)
{
    return std::tan(fov * 0.5f) / (size * 0.5f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Camera

Camera make_camera(Orbit const& orbit, Zoom const& zoom)
{
    Camera cam{};
    orbit.apply(cam);
    zoom.apply(cam);
    return cam;
}

Camera camera_front(Vec3<f32> const& target, f32 const offset)
{
    Camera cam{};

    Mat3<f32> m;
    m.col(0) = Vec3<f32>::UnitY();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = Vec3<f32>::UnitX();

    cam.pivot.rotation = Quat<f32>{m};
    cam.pivot.position = target;
    cam.offset.z() = offset;
    return cam;
}

Camera camera_back(Vec3<f32> const& target, f32 const offset)
{
    Camera cam{};

    Mat3<f32> m;
    m.col(0) = -Vec3<f32>::UnitY();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = -Vec3<f32>::UnitX();

    cam.pivot.rotation = Quat<f32>{m};
    cam.pivot.position = target;
    cam.offset.z() = offset;
    return cam;
}

Camera camera_look_at(
    Vec3<f32> const& position,
    Vec3<f32> const& target,
    Vec3<f32> const& up)
{
    Camera cam{};
    cam.pivot.position = target;

    Vec3<f32> const d = position - target;
    f32 const d_norm = d.norm();

    // Create pivot rotation
    Mat3<f32> m;
    m.col(2) = d / d_norm;
    m.col(0) = up.cross(m.col(2)).normalized();
    m.col(1) = m.col(2).cross(m.col(0));

    // Assign rotation and offset
    cam.pivot.rotation = Quat<f32>{m};
    cam.offset.z() = d_norm;

    return cam;
}

void zoom_to_fit(Camera& camera, Vec3<f32> const& center, f32 const radius, f32 const fov_min)
{
    camera.pivot.position = center;
    camera.offset = {0.0f, 0.0f, zoom_distance(radius, fov_min)};
}

f32 zoom_distance(f32 const radius, f32 const fov_min)
{
    return radius / std::sin(fov_min * 0.5f);
}

void camera_handle_mouse(
    Camera& camera,
    Orbit* const orbit,
    Zoom* const zoom,
    Pan* const pan,
    bool* const mouse_down,
    f32 const move_scale,
    sapp_event const& event)
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
                orbit->handle_drag(camera, {event.mouse_dx * move_scale, event.mouse_dy * move_scale});

            if (mouse_down[1] && pan != nullptr)
                pan->handle_drag(camera, {event.mouse_dx * move_scale, event.mouse_dy * move_scale});

            break;
        }
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
        {
            constexpr f32 scroll_scale = 0.1f; // TODO: Expose this?

            if (zoom != nullptr)
                zoom->handle_scroll(camera, sign(event.scroll_y) * scroll_scale);

            break;
        }
        default:
        {
            // ...
        }
    }
}

void camera_handle_touch(
    Camera& camera,
    Orbit* const orbit,
    Zoom* const zoom,
    Pan* const /*pan*/,
    Vec2<f32>* const last_touch_points,
    i8& last_num_touches,
    f32 const move_scale,
    sapp_event const& event)
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
                            orbit->handle_drag(camera, d * move_scale);
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
                            zoom->handle_scroll(camera, (d1 - d0) * move_scale);
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// Transforms

Mat4<f32> scale(Vec3<f32> const& scale)
{
    Mat4<f32> m = Mat4<f32>::Identity();
    m.diagonal().head<3>() = scale;
    return m;
}

Mat4<f32> translate(Vec3<f32> const& translate)
{
    Mat4<f32> m = Mat4<f32>::Identity();
    m.col(3).head<3>() = translate;
    return m;
}

Mat4<f32> scale_translate(Vec3<f32> const& scale, Vec3<f32> const& translate)
{
    Mat4<f32> m{};
    m.diagonal().head<3>() = scale;
    m.col(3) = translate.homogeneous();
    return m;
}

Mat3<f32> ortho_basis(Vec3<f32> const& unit_x, Vec3<f32> const& xy)
{
    Vec3<f32> const unit_z = unit_x.cross(xy).normalized();

    Mat3<f32> m;
    m.col(0) = unit_x;
    m.col(1) = unit_z.cross(unit_x);
    m.col(2) = unit_z;
    return m;
}

Mat4<f32> as_affine(Mat3<f32> const& linear)
{
    Mat4<f32> m = Mat4<f32>::Identity();
    m.topLeftCorner<3, 3>() = linear;
    return m;
}

Mat4<f32> look_at(Vec3<f32> const& eye, Vec3<f32> const& target, Vec3<f32> const& up)
{
    Vec3<f32> const z = (eye - target).normalized();
    Vec3<f32> const x = up.cross(z).normalized();
    Vec3<f32> const y = z.cross(x);

    Mat4<f32> m;
    m.col(0) << x.x(), y.x(), z.x(), 0.0f;
    m.col(1) << x.y(), y.y(), z.y(), 0.0f;
    m.col(2) << x.z(), y.z(), z.z(), 0.0f;
    m.col(3) << -eye.dot(x), -eye.dot(y), -eye.dot(z), 1.0f;
    return m;
}

Mat2<f32> rotate(f32 const angle)
{
    f32 const c = std::cos(angle);
    f32 const s = std::sin(angle);

    Mat2<f32> m;
    m.col(0) << c, s;
    m.col(1) << -s, c;
    return m;
}

Vec4<f32> to_plane_eqn(Vec3<f32> const& origin, Vec3<f32> const& normal)
{
    return {
        normal.x(),
        normal.y(),
        normal.z(),
        -(normal.dot(origin))};
}

template <>
Mat4<f32> perspective<NdcType_Default>(f32 const fov_y, f32 const aspect, f32 const near, f32 const far)
{
    // NOTE: This assumes a right-handed y-up view space i.e. [-near, -far] in view space maps to
    // [-1, 1] in NDC space

    f32 const y = std::tan(fov_y * 0.5f);

    Mat4<f32> m;
    m.col(0) << 1.0f / (y * aspect), 0.0f, 0.0f, 0.0f;
    m.col(1) << 0.0f, 1.0f / y, 0.0f, 0.0f;
    m.col(2) << 0.0f, 0.0f, far / (near - far), -1.0f;
    m.col(3) << 0.0f, 0.0f, -far * near / (far - near), 0.0f;

    return m;
}

template <>
Mat4<f32> perspective<NdcType_OpenGl>(f32 const fov_y, f32 const aspect, f32 const near, f32 const far)
{
    // NOTE: This assumes a right-handed y-up view space i.e. [-near, -far] in view space maps to
    // [-1, 1] in NDC space

    f32 const y = std::tan(fov_y * 0.5f);

    Mat4<f32> m;
    m.col(0) << 1.0f / (y * aspect), 0.0f, 0.0f, 0.0f;
    m.col(1) << 0.0f, 1.0f / y, 0.0f, 0.0f;
    m.col(2) << 0.0f, 0.0f, -(far + near) / (far - near), -1.0f;
    m.col(3) << 0.0f, 0.0f, -2.0f * far * near / (far - near), 0.0f;

    return m;
}

template <>
Mat4<f32> perspective<NdcType_Vulkan>(f32 const fov_y, f32 const aspect, f32 const near, f32 const far)
{
    // NOTE: This assumes a right-handed y-up view space i.e. [-near, -far] in view space maps to
    // [0, 1] in NDC space

    f32 const y = std::tan(fov_y * 0.5f);

    Mat4<f32> m;
    m.col(0) << 1.0f / (y * aspect), 0.0f, 0.0f, 0.0f;
    m.col(1) << 0.0f, -1.0f / y, 0.0f, 0.0f;
    m.col(2) << 0.0f, 0.0f, far / (near - far), -1.0f;
    m.col(3) << 0.0f, 0.0f, -far * near / (far - near), 0.0f;

    return m;
}

template <>
Mat4<f32> orthographic<NdcType_Default>(
    f32 const left,
    f32 const right,
    f32 const bottom,
    f32 const top,
    f32 const near,
    f32 const far)
{
    // NOTE: This assumes a right-handed y-up view space i.e. [-near, -far] in view space maps to
    // [0, 1] in NDC space

    f32 const inv_w = 1.0f / (right - left);
    f32 const inv_h = 1.0f / (top - bottom);
    f32 const inv_d = 1.0f / (near - far);

    Mat4<f32> m;
    m.col(0) << 2.0f * inv_w, 0.0f, 0.0f, 0.0f;
    m.col(1) << 0.0f, 2.0f * inv_h, 0.0f, 0.0f;
    m.col(2) << 0.0f, 0.0f, inv_d, 0.0f;
    m.col(3) << -(right + left) * inv_w, -(top + bottom) * inv_h, near * inv_d, 1.0f;

    return m;
}

template <>
Mat4<f32> orthographic<NdcType_OpenGl>(
    f32 const left,
    f32 const right,
    f32 const bottom,
    f32 const top,
    f32 const near,
    f32 const far)
{
    // NOTE: This assumes a right-handed y-up view space i.e. [-near, -far] in view space maps to
    // [-1, 1] in NDC space

    f32 const inv_w = 1.0f / (right - left);
    f32 const inv_h = 1.0f / (top - bottom);
    f32 const inv_d = 1.0f / (near - far);

    Mat4<f32> m;
    m.col(0) << 2.0f * inv_w, 0.0f, 0.0f, 0.0f;
    m.col(1) << 0.0f, 2.0f * inv_h, 0.0f, 0.0f;
    m.col(2) << 0.0f, 0.0f, 2.0f * inv_d, 0.0f;
    m.col(3) << -(right + left) * inv_w, -(top + bottom) * inv_h, (far + near) * inv_d, 1.0f;

    return m;
}

template <>
Mat4<f32> orthographic<NdcType_Vulkan>(
    f32 const left,
    f32 const right,
    f32 const bottom,
    f32 const top,
    f32 const near,
    f32 const far)
{
    // NOTE: This assumes a right-handed y-up view space i.e. [-near, -far] in view space maps to
    // [0, 1] in NDC space

    f32 const inv_w = 1.0f / (right - left);
    f32 const inv_h = 1.0f / (top - bottom);
    f32 const inv_d = 1.0f / (near - far);

    Mat4<f32> m;
    m.col(0) << 2.0f * inv_w, 0.0f, 0.0f, 0.0f;
    m.col(1) << 0.0f, -2.0f * inv_h, 0.0f, 0.0f;
    m.col(2) << 0.0f, 0.0f, inv_d, 0.0f;
    m.col(3) << -(right + left) * inv_w, (top + bottom) * inv_h, near * inv_d, 1.0f;

    return m;
}

} // namespace dr