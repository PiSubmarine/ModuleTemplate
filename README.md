# ModuleTemplate

`ModuleTemplate` is the starting point for most non-STM32 PiSubmarine modules. It shows the expected repository layout, the standard CMake entry points, and the default integration with the PiSubmarine build system and `vcpkg`.

## Folder structure

PiSubmarine modules follow a strict layout. The important folders are:

- `src`: the main library implementation of the module. Reusable production code lives here and is compiled into the module library.
- `public`: public headers for code in `src`. Other modules include headers from here and link against the library built from `src`.
- `app`: executables that are closely related to the module. These are useful for composition roots, tools, demos, or local manual testing, but they are not intended to be linked as dependencies by other modules.
- `test`: unit tests for code in `src`. Tests verify behavior and can use `gtest` together with test-local `gmock` code that is not part of the module's public test surface.
- `mock`: public `gmock`-based mocks, typically for interfaces defined in `public`. Other modules can link these mocks into their own tests.
- `doc`: optional module-specific documentation.

The recommended convention is to mirror the namespace in the directory structure. For example, a class in namespace `PiSubmarine::ModuleTemplate` is typically placed like this:

```text
public/PiSubmarine/ModuleTemplate/ModuleTemplate.h
src/PiSubmarine/ModuleTemplate/ModuleTemplate.cpp
test/PiSubmarine/ModuleTemplate/ModuleTemplateTest.cpp
mock/PiSubmarine/ModuleTemplate/IModuleTemplateMock.h
app/PiSubmarine/ModuleTemplate/main.cpp
```

## Build system

The top-level [`CMakeLists.txt`](./CMakeLists.txt) bootstraps the shared PiSubmarine build logic from [`Build.CMake`](https://github.com/PiSubmarine/Build.CMake). Most module-specific dependency work happens in subdirectory `CMakeLists.txt` files, usually in `src/CMakeLists.txt`.

### Internal PiSubmarine dependencies

Internal dependencies are added with `PiSubmarineAddDependency(URL TAG)` and then linked in the normal CMake way.

Example:

```cmake
PiSubmarineAddDependency("https://github.com/PiSubmarine/Udp.Api.git" "")
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.Udp.Api)
```

Typical usage pattern:

1. Add `PiSubmarineAddDependency(...)` in the `CMakeLists.txt` of the target that needs the dependency, usually `src/CMakeLists.txt`.
2. Link the fetched module with `target_link_libraries(...)`.
3. Include the dependency's public headers from your code.

Notes:

- Use the GitHub repository URL, not a path into `_deps`, `out/build`, or another generated directory.
- Do not modify dependencies inside the build directory. `FetchContent` creates generated copies there and local edits may be lost.
- If you need to change another PiSubmarine module, modify that module in its own repository and then update the `TAG` in `PiSubmarineAddDependency(...)` if your module is pinned to a specific revision.
- An empty tag such as `""` means the current default branch is used. If reproducibility matters, prefer pinning to a tag or commit.

### Configuring and building

Always use CMake presets from [`CMakePresets.json`](./CMakePresets.json). Do not invoke CMake with ad-hoc parameters.

The main supported platform families in PiSubmarine are:

- Raspberry Pi / Linux
- Windows
- STM32, but only for repositories that contain STM32-specific code

`ModuleTemplate` itself is intended for the usual non-STM32 module layout. STM32 repositories often need a different structure and additional platform-specific build files.

This template currently defines these configure presets:

- `windows-msvc-debug`: native Windows build with MSVC
- `linux-wsl2-debug`: Linux x64 build in WSL2
- `pisubmarine-wsl2-debug`: cross-build for Raspberry Pi (`aarch64` Linux) from WSL2
- `windows-wsl2-debug`: cross-build for Windows from WSL2 using MinGW
- `pisubmarine-ssh-debug`: remote configuration preset for PiSubmarine SSH tooling

Typical commands:

```powershell
cmake --preset windows-msvc-debug
cmake --build out/build/windows-msvc-debug
ctest --test-dir out/build/windows-msvc-debug --output-on-failure
```

```powershell
cmake --preset linux-wsl2-debug
cmake --build out/build/linux-wsl2-debug
ctest --test-dir out/build/linux-wsl2-debug --output-on-failure
```

```powershell
cmake --preset pisubmarine-wsl2-debug
cmake --build out/build/pisubmarine-wsl2-debug
ctest --test-dir out/build/pisubmarine-wsl2-debug --output-on-failure
```

Use the preset that matches your platform and toolchain.

### Windows path length warning

Builds on Windows may fail because of path length limitations, especially after dependencies are fetched into deep build directory trees.

The only known workaround for now is to shorten the Windows build and install directories in the Windows CMake preset, for example:

```json
"binaryDir": "C:/t/b",
"installDir": "C:/t/i"
```

This change should be applied in the Windows preset definition in [`CMakePresets.json`](./CMakePresets.json).

## External dependencies with vcpkg

External libraries are declared in [`vcpkg.json`](./vcpkg.json). The template already includes:

```json
{
  "dependencies": [
    "spdlog",
    "gtest"
  ]
}
```

To add a new external dependency:

1. Add the package name to `vcpkg.json`.
2. Reconfigure the project with the appropriate CMake preset.
3. Use `find_package(...)` in the relevant `CMakeLists.txt`.
4. Link the imported target with `target_link_libraries(...)`.

Example:

```json
{
  "dependencies": [
    "spdlog",
    "gtest",
    "fmt"
  ]
}
```

```cmake
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PISUBMARINE_MODULE_NAME} PRIVATE fmt::fmt)
```

Notes:

- `vcpkg-configuration.json` defines the registry configuration and baseline used by the module.
- Keep external dependencies minimal. If the dependency is a PiSubmarine module, use `PiSubmarineAddDependency(...)` instead of `vcpkg`.
- Test-only packages should only be linked from test or mock targets unless production code genuinely needs them.

## Other important notes

- Put reusable logic in `src` and `public`, not in `app`.
- Keep module responsibility narrow. If a dependency or feature pushes the module beyond one clear purpose, consider splitting it.
- Prefer depending on PiSubmarine `*.Api` modules from implementation modules.
- For recoverable failures, prefer `PiSubmarine::Error::Api::Result<T>` from [`Error.Api`](../Error.Api/public/PiSubmarine/Error/Api/Result.h).
- Business logic should remain testable without real hardware.
