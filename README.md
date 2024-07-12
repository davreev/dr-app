# dr-app

![](https://github.com/davreev/dr-app/actions/workflows/build.yml/badge.svg)

Cross-platform scaffolding for simple 3D applications via [Sokol](https://github.com/floooh/sokol) and [Dear ImGui](https://github.com/ocornut/imgui)

> ⚠️ This libray exists mainly to cut down on boilerplate when prototyping. It's likely to undergo
> frequent breaking changes.

## Build

Use CMake to build

> ⚠️ Currently only tested with Clang and GCC. MSVC is not supported.

```sh
mkdir build

# If using a single-config generator (e.g. Unix Makefiles, Ninja)
cmake -S . -B ./build -DCMAKE_BUILD_TYPE=(Debug|Release|RelWithDebInfo) [-DDR_APP_EXAMPLE=ON] [-DDR_APP_TEST=ON]
cmake --build ./build

# If using a multi-config generator (e.g. Ninja Multi-Config, Xcode)
cmake -S . -B ./build [-DDR_APP_EXAMPLE=ON] [-DDR_APP_TEST=ON]
cmake --build ./build --config (Debug|Release|RelWithDebInfo)
```

## Usage

Use `FetchContent` to include in another CMake build

```cmake
include(FetchContent)

FetchContent_Declare(
    dr-app
    GIT_REPOSITORY https://github.com/davreev/dr-app.git
    GIT_TAG <revision>
)

# Creates target "dr::app"
FetchContent_MakeAvailable(dr-app)

# Link other targets e.g.
add_executable(my-app main.cpp)
target_link_libraries(my-app PRIVATE dr::app)
```
