## ioq3-renderer-bgfx

[![Appveyor CI Build Status](https://ci.appveyor.com/api/projects/status/github/jpcy/ioq3-renderer-bgfx?branch=master&svg=true)](https://ci.appveyor.com/project/jpcy/ioq3-renderer-bgfx)
[![Travis CI Build Status](https://travis-ci.org/jpcy/ioq3-renderer-bgfx.svg?branch=master)](https://travis-ci.org/jpcy/ioq3-renderer-bgfx) [![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

This is a renderer for [ioquake3](https://github.com/ioquake/ioq3) that uses [bgfx](https://github.com/bkaradzic/bgfx) to support multiple graphics APIs.

Minimum requirements: OpenGL 3.2 or Direct3D 11.

## Features
* Anti-aliasing - MSAA, SMAA
* Soft sprites
* Real dynamic lights, with extra dynamic lights for Q3A weapons - BFG, Lightning, Plasma, Railgun
* Bloom

## Screenshots

| Bloom | Extra Dynamic Lights |
|---|---|
| [![](http://i.imgur.com/86x8FN2.png)](http://i.imgur.com/WHYjbF0.jpg) | [![](http://i.imgur.com/eA2ydm8.png)](http://i.imgur.com/vPhQbMc.jpg) |

| Planar Reflections | Soft Sprites |
|---|---|
| [![](http://i.imgur.com/KkGO5Hc.png)](http://i.imgur.com/ShxFR3o.jpg) | [![](http://i.imgur.com/1QPNbzr.png)](http://i.imgur.com/LvMyLgB.jpg) |

## Binaries

| [Windows (x86)](https://bintray.com/jpcy/ioq3-renderer-bgfx/windows/test#files) | [Linux (x86_64)](https://bintray.com/jpcy/ioq3-renderer-bgfx/linux/test#files) |
|---------------------------------------------------------------------------------|--------------------------------------------------------------------------------|

These are updated after every commit.

## Compiling

### Linux

Required packages: libgl1-mesa-dev libsdl2-dev

```
./premake5 shaders
./premake5 gmake
cd build
make
```

### Cygwin/MinGW-w64/MSYS2

Clone [ioquake3](https://github.com/ioquake/ioq3) to the same parent directory as ioq3-renderer-bgfx.

```
./premake5.exe shaders
./premake5.exe gmake
cd build
make
```

### Visual Studio

1. Run `CompileShaders.bat`
2. Use [ioq3-premake-msvc](https://github.com/jpcy/ioq3-premake-msvc).

## Usage

Copy the renderer binaries from `build\bin_*` to where you have a [ioquake3 test build](http://ioquake3.org/get-it/test-builds/) installed.

Select the renderer in-game with `cl_renderer bgfx` followed by `vid_restart`.

### Console Variables

Run the following console variables without any arguments to see a list of possible values.

Variable                | Description
------------------------|------------
r_aa                    | Anti-aliasing.
r_backend               | Rendering backend - OpenGL, Direct3D 9 etc.
r_bgfx_stats            | Show bgfx statistics.
r_bloom                 | Enable bloom.
r_bloomScale            | Scale the bloom effect.
r_dynamicLightIntensity | Make dynamic lights brighter/dimmer.
r_dynamicLightScale     | Scale the radius of dynamic lights.
r_extraDynamicLights    | Enable extra dynamic lights on Q3A weapons.
r_fastPath              | Disables all optional features to improve performance.
r_lerpTextureAnimation  | Use linear interpolation on texture animation - flames, explosions.
r_maxAnisotropy         | Enable [anisotropic filtering](https://en.wikipedia.org/wiki/Anisotropic_filtering).
r_textureVariation      | Hide obvious texture tiling in a few Q3A maps.
r_waterReflections      | Show planar water reflections. Only enabled on q3dm2 for now.

### Console Commands

Command        | Description
---------------|------------
r_captureFrame | Capture a RenderDoc frame.
screenshotPNG  |

## RenderDoc

The renderer must be built in debug mode - `make config=debug_x86` or `make config=debug_x86_64`.

Place the RenderDoc shared library - `renderdoc.dll` or `librenderdoc.so` - in the same directory as the renderer binary.

Use the `r_captureFrame` console command to capture a frame. Bind it to a key so the console doesn't show up in the capture.

Capture files will be saved to the same directory as the ioquake3 executable as `ioq3-renderer-bgfx_frameX.rdc` where X is the frame number.
