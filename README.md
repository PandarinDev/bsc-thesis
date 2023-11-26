# Infinitown
This repository contains my Bachelor's thesis project which is a software project for procedurally generating infinite towns using the "Wave Function Collapse" algorithm and displaying them real-time using Vulkan.

# OS Support
Windows and Linux should be supported out of the box, without tinkering (provided that you have a compatible GPU and relatively recent GPU drivers installed).

Some amount of tinkering is required on MacOS however, as Vulkan is not natively available there and we need to use MoltenVK as a compatibility layer.
Please ensure that you have the `molten-vk` and `vulkan-loader` packages installed: `brew install molten-vk vulkan-loader`
When running the application the `libvulkan.1.dylib` library must be on your `DYLD_LIBRARY_PATH`, i.e. start the application by running: `DYLD_LIBRARY_PATH=/opt/homebrew/lib ./build/infinitown`.
