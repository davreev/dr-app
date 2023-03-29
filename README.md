# dr-app

## Build

Build with CMake

> ⚠️ Currently only tested with Clang and GCC. MSVC is not supported.

```sh
mkdir build
cd build

# If using a single-config generator (e.g. Unix Makefiles, Ninja)
cmake -DCMAKE_BUILD_TYPE=(Debug|Release|RelWithDebInfo) [-DDR_APP_EXAMPLE=ON] ..
cmake --build .

# If using a multi-config generator (e.g. Ninja Multi-Config, Xcode)
cmake [-DDR_APP_EXAMPLE=ON] ..
cmake --build . --config (Debug|Release|RelWithDebInfo)
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
