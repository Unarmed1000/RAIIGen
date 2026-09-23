# RAIIGen
Scans the Vulkan, OpenCL, OpenGLES and OpenVX headers with libclang and generates C++ RAII classes for the API handles.

It is used to generate [RapidVulkan](https://github.com/Unarmed1000/RapidVulkan), which targets C++17.
The OpenCL, OpenGLES and OpenVX generators still produce the older C++11 style classes and are currently disabled in [Main.cpp](source/RAIIGen/Main.cpp).

The project started as an experiment with libclang, so the goal is output that is 'good enough' rather than a polished general purpose tool.

## Building
Requirements:
* Visual Studio 2026 (MSVC v145) with C++20, 64-bit only.
* [LLVM](https://github.com/llvm/llvm-project/releases) for libclang, installed to `C:\Program Files\LLVM` by default.

Build with CMake (4.0 or newer)
```
cmake --preset vs2026
cmake --build --preset vs2026-release
```
Use `-DLLVM_ROOT=<path>` if LLVM is installed somewhere else. A `ninja` preset is also available when running from a x64 developer prompt.
The build copies `libclang.dll` next to the executable, so LLVM does not need to be on the PATH.

Or open [RAIIGen.slnx](RAIIGen.slnx) in Visual Studio 2026.

## Running
Run the executable from the repository root, it reads everything from `config/` and writes the generated code to `output/`.

```
build\vs2026\Release\RAIIGen.exe
```

Directory                               | Content
----------------------------------------|------------------------
config/Headers/khronos/&lt;API&gt;         | The API headers that are scanned.
config/Headers/khronos/Vulkan1.0/history | One directory per Vulkan header release. The newest release is used for the generated code and the older ones are used to add `VK_HEADER_VERSION` guards.
config/Templates/&lt;Name&gt;              | The templates and snippets that control the generated code.
output/&lt;Name&gt;&lt;Version&gt;          | The generated code, for example `output/RapidVulkan1.0`.

### Adding a new Vulkan release
1. Create `config/Headers/khronos/Vulkan1.0/history/<sdk version>` (for example `1.4.357.0`).
2. Copy the `vulkan` and `vk_video` directories from the Vulkan SDK `Include` directory into it.
3. Run RAIIGen and check the log for `WARNING: Missing default value for type` messages.
   New handle types need a default value in `g_typeDefaultValues` in [VulkanGenerator.cpp](source/RAIIGen/Generator/VulkanGenerator.cpp).

### Updating RapidVulkan
Replace `include/RapidVulkan` in the RapidVulkan repository with the content of `output/RapidVulkan1.0`, except the `Vk` directory which is not part of RapidVulkan.
Then update the version in its `CMakeLists.txt` to match the Vulkan SDK version.

## Third party code
* [fmt](https://github.com/fmtlib/fmt) 12.2.0 in `include/fmt` and `source/fmt`.
* FslBase from the [gtec-demo-framework](https://github.com/NXPmicro/gtec-demo-framework) in `include/FslBase` and `source/FslBase`.
  It is kept identical to the upstream version, so it can be updated by copying the upstream directories.

## Rules for forks and generated code
All forked versions of the code must include the original "##AG_TOOL_STATEMENT##" line in the generated code.
This means that all generated code contains a line like this:

```
// Auto-generated Vulkan 1.0 C++17 RAII classes by RAIIGen (https://github.com/Unarmed1000/RAIIGen)
```

The exact format depends on which API and tool version was used.

All released auto generated files must include the corresponding 'RAIIGenVersion.txt' file.
All forked versions of the code must leave the original first line of 'RAIIGenVersion.txt' intact, but are free to add additional lines to the file.

LICENSE: BSD 3-Clause License
