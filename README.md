# SA:MP Launcher

An improved, modernized SA:MP 0.3.7/DL launcher - rewritten in C++, adapted for RAD Studio, and fully compatible with the original launcher

## Highlights

- migrated to RAD Studio
- fully rewritten from Pascal to C++
- fixed numerous source-code and VCL form issues
- updated icons with higher-resolution versions
- added full High DPI support
- added 0.3.DL features
- improved compatibility, stability, and security
- fixed and improved `samp.dll` injection into `gta_sa.exe`
- fixed server and RCON password save
- fixed UI elements disappearing after pressing Alt
- Internet and Hosted tabs now work, using `api.open.mp` as the master server source
- added RCON menu item (was present in older versions)
- made the launcher multithreaded and asynchronous throughout

## Structure

```text
source/    C++ source files and headers
dfm/       VCL form files
resource/  icon, manifest, images
```

## Build

Requirements: CMake, Ninja, RAD Studio with TeeChart component (tested on RAD Studio 11.3)

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
