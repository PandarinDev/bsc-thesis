# Infinitown
This repository contains my Bachelor's thesis project which is a software project for procedurally generating infinite towns using the "Wave Function Collapse" algorithm and displaying them real-time using Vulkan.

## Building and Running
A C++17 compatible compiler is required to build the application (both `gcc` and `clang` are supported, others are untested) as well as a recent version of CMake (version 3.20 or newer).
Currently shaders are built on the host, and thus are only assumed to be compatible with the host hardware, so `glslc` is required to be on PATH. It is part of the VulkanSDK binary toolset.
In order to build the application run `.\build.bat` on Windows or `./build.sh` on Linux and MacOS. The binaries will be created inside the `build` folder.

To launch the application look for the `infinitown` executable inside the build folder.

## Running Tests
There is a unit test harness provided with the application to ensure correctness. In order to run the unit tests look for the binary `infinitown-tests` in the `build/tests` folder.

## OS Support
### Windows & Linux
Windows and Linux should be supported out of the box, provided that you have a Vulkan compatible GPU and relatively recent GPU drivers installed.

### MacOS
Since Vulkan is not available on MacOS, we use MoltenVK as a compatibility layer to translate Vulkan calls and shaders to Metal.  
Please ensure that you have the `molten-vk` and `vulkan-loader` packages installed: `brew install molten-vk vulkan-loader`  
When running the application the `libvulkan.1.dylib` library must be on your `DYLD_LIBRARY_PATH`, i.e. start the application by running: `DYLD_LIBRARY_PATH=/opt/homebrew/lib ./build/infinitown`.
