#include <dr/math_constants.hpp>
#include <dr/math_types.hpp>

#include <dr/app/camera_rig.hpp>

namespace dr
{

struct Pan
{
    struct
    {
        Vec2<f32> current;
        Vec2<f32> target{current};
    } offset;
    f32 sensitivity{1.0};

    void set(CameraRig const& rig);
    void set_target(CameraRig const& rig) { offset.target = rig.offset.head<2>(); }
    void apply(CameraRig& rig) const { rig.offset.head<2>() = offset.current; }
    void update(f32 const t) { offset.current += (offset.target - offset.current) * t; }
    void handle_input(Vec2<f32> const& delta);
};

struct Zoom
{
    struct
    {
        f32 current{1.0};
        f32 target{current};
        f32 min{};
        f32 max{1.0e5};
    } distance;
    f32 sensitivity{1.0};

    void set(CameraRig const& rig);
    void set_target(CameraRig const& rig) { distance.target = rig.offset.z(); }
    void apply(CameraRig& rig) const { rig.offset.z() = distance.current; }
    void update(f32 const t) { distance.current += (distance.target - distance.current) * t; }
    void handle_input(f32 delta);
};

struct Orbit
{
    struct
    {
        f32 current{pi<f32> * 0.5f};
        f32 target{current};
        f32 min{};
        f32 max{pi<f32>};
    } polar;
    struct
    {
        f32 current{};
        f32 target{current};
    } azimuth;
    f32 sensitivity{5.0};

    void set(CameraRig const& rig);
    void set_target(CameraRig const& rig);
    void apply(CameraRig& rig) const;
    void update(f32 t);
    void handle_input(Vec2<f32> const& delta);
};

/// Creates a camera rig from the given controls
CameraRig make_camera(Orbit const& orbit, Zoom const& zoom);

/// Creates a camera rig from the given controls
CameraRig make_camera(Orbit const& orbit, Zoom const& zoom, Pan const& pan);

} // namespace dr