# dr-app

![](https://github.com/davreev/dr-app/actions/workflows/build.yml/badge.svg)

## Build

Build with CMake

> ⚠️ Currently only tested with Clang and GCC. MSVC is not supported.

```sh
mkdir build

# If using a single-config generator (e.g. Unix Makefiles, Ninja)
cmake -S . -B ./build -DCMAKE_BUILD_TYPE=(Debug|Release|RelWithDebInfo) [-DDR_APP_EXAMPLE=ON]
cmake --build ./build

# If using a multi-config generator (e.g. Ninja Multi-Config, Xcode)
cmake -S . -B ./build [-DDR_APP_EXAMPLE=ON]
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
