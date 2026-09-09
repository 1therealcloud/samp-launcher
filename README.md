# SA:MP Launcher

Modernized SA:MP launcher for Windows.

## Highlights

- migrated to RAD Studio 11.2
- fully rewritten from Pascal to C++
- fixed numerous source-code and VCL form issues
- updated icons with higher-resolution versions
- added full High DPI support
- improved Windows compatibility, stability, and security
- fixed and improved `samp.dll` injection into `gta_sa.exe`
- fixed server and RCON password save
- fixed UI elements disappearing after pressing Alt
- restored Internet and Hosted tabs
- restored and improved the **Master Server Update** button
- made Internet/Hosted updates faster, asynchronous, and multithreaded
- preserved compatibility with the original launcher
- added a visible RCON connection menu item
- runs the RCON console directly from the launcher

## Structure

```text
source/    C++ source files and headers
dfm/       VCL form files
resource/  icon, manifest, images
```

## Build

Requirements: CMake, Ninja, and RAD Studio 11.2

Dynamic build:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM="PATH_TO_NINJA" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Static build:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM="PATH_TO_NINJA" -DCMAKE_BUILD_TYPE=Release -DSAMP_STATIC=ON
cmake --build build --config Release
```
