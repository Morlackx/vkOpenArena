
    ,---------------------------------------------------------------------------.
    |    ______                                                                 |
    |   / ____ \                            /\                                  |
    |  / /    \ \                          //\\                                 |
    | / /      \ \  _____  ____    _____  //  \\    _   _  ____    _____ ____   |
    | \ \      / / //  // //  \\  //  // //====\\   || // //  \\  //  // ___\\  |
    |  \ \____/ / //__//  \\__// //  // //      \\  ||//  \\__// //  // //   \\ |
    |   \______/ //        \___ //  // //        \\ ||     \___ //  //  \\___// |
    |                                                                           |
    `---------------------------- http://openarena.ws --------------------------'


# OpenArena Engine 
This project is a fork of OpenArena with specific changes to its renderer module.
I am naive programmer, this repository is mainly for myself learning Vulkan and Quake3's engine.
I actually haven't the ability to maintain it as lacking knowledge about game graphics.
I'm learning by modification, however, only to find that I introduce bugs and mess the code up.
To keep this code alive, it needs your help, any instructions would be appreciated. 

For people who want to try the vulkan based renderer on quake3's map,
go to https://github.com/suijingfeng/vkQuake3

![](https://github.com/suijingfeng/vkOpenArena/blob/master/doc/quake3.jpg)

## Building on Ubuntu or Debian Linux ##

Install the build dependencies.

```sh
$ sudo apt-get install libcurl4-openssl-dev libsdl2-dev libopenal-dev libvulkan-dev libgl1-mesa-dev
$ sudo apt-get install clang gcc make git
$ git clone https://github.com/suijingfeng/vkOpenArena.git
$ cd vkOpenArena
$ make -j4
```

Please note that vulkan renderer requires at least SDL 2.0.6. 
The precompiled versions in some of the distribute repositories 
do not ship with Vulkan support, you therefore may come cross the
following problem when you launch OA:

```
Vulkan support is either not configured in SDL or not available in video driver.
```
This problem can be solved easily by compiling the SDL 2.0.9 from source:

Get a copy of the source code from https://www.libsdl.org/, then extract it.

```
cd SDL
mkdir build
cd build
../configure
make -j4
sudo make install
```


## Building on Windows 7 or 10 ##

To build 64-bit binaries, follow these instructions:

1. Install msys2 from https://msys2.github.io/ , following the instructions there.
2. Start "MinGW 64-bit" from the Start Menu, NOTE: NOT MSYS2.
3. Install mingw-w64-x86\_64, make, git and necessary libs.
```sh
pacman -S mingw-w64-x86_64-gcc make git
```
4. Grab latest openarena source code from github and compile. Note that in msys2, your drives are linked as folders in the root directory: C:\ is /c/, D:\ is /d/, and so on.

```sh
git clone https://github.com/suijingfeng/vkOpenArena.git
cd vkOpenArena
make -j4
```
5. Find the executables and dlls in build/release-mingw64-x86\_64 . 



## RUN ##
First, download the map packages from http://openarena.ws/download.php
Second, extract the data files at ~/.OpenArena/ (on linux) 
C:\Users\youname\AppData\Roaming\OpenArena\ (on windows)


```sh
$ cd /build/release-linux-x86_64/
$ ./openarena.x86_64
```


## Switching renderers ##


This feature is enabled by default. This allow for build modular renderers and select or switch 
the renderer at runtime rather than compiling into one binary.
If you wish to disable it, set `USE_RENDERER_DLOPEN=0` in the Makefile.
When you start OpenArena, you can switch witch dynamic library to load by passing its name. 

Example:

```sh

# New vulkan renderer backend, under developing, 
# work on ubuntu 18.04, ubuntu16.04, win10, win7.
# associate code located in code/renderer_vulkan, see readme there.
$ ./openarena.x86_64 +set cl_renderer vulkan

# Enable renderergl2( borrowed from ioq3 ):
$ ./openarena.x86_64 +set cl_renderer opengl2

# Enable the renderergl1( borrowed from ioq3 ):
$ ./openarena.x86_64 +set cl_renderer opengl1

# Enable the mydev( borrowed from Kenny):
# This renderer module is similiar to the renderergl1's code.
# However, its seem run faster even than vulkan, 
# I got a good feeling play OA with this renderer enable.
# I don't known the reason why.
$ ./openarena.x86_64 +set cl_renderer mydev


# Enable the default OpenArena renderer:
# This renderer module is similiar to the renderergl1 code.
$ ./openarena.x86_64 +set cl_renderer openarena
```

Q: How to enable vulkan support from the pulldown console ?
```sh
\cl_renerer vulkan
\vid_restart
```
Q: How to check that Vulkan backend is really active ? 
```sh
\vkinfo
```
Type \vkinfo in the console reports information about active rendering backend.
It will report something as following:

```
Active 3D API: Vulkan
Vk api version: 1.0.65
Vk driver version: 1637679104
Vk vendor id: 0x10DE (NVIDIA)
Vk device id: 0x1B80
Vk device type: DISCRETE_GPU
Vk device name: GeForce GTX 1080

Total Device Extension Supported:

...

Vk instance extensions:

...

Image chuck memory(device local) used: 8 M 

```
You can also get the information from the UI: SETUP >> SYSTEM >> DRIVER INFO

![](https://github.com/suijingfeng/vkOpenArena/blob/master/doc/driver_info.jpg)

# OpenArena gamecode

## Description
It's non engine part of OA, includes game, cgame and ui.
In mod form it is referred as OAX. 

## Loading native dll(.so)

```
cd linux_scripts/
./supermake_native
```


## Links
Development documentation is located here: https://github.com/OpenArena/gamecode/wiki

The development board on the OpenArena forum: http://openarena.ws/board/index.php?board=30.0

In particular the Open Arena Expanded topic: http://openarena.ws/board/index.php?topic=1908.0



## Status

* Initial testing on Ubuntu16.04 and Ubuntu18.04, Win7 and Win10


## License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

## bugs/issues

* About com\_hunkmegs

When i playing CTF on :F for stupid server with default com\_hunkmegs = 128 setting, the following errors occurs:
```
ERROR: Hunk_Alloc failed on 739360: code/renderergl2/tr_model.c, line: 535 (sizeof(*v) * (md3Surf->numVerts * md3Surf->numFrames)).
```
OpenGL2 renderer seems use more memory, Upping com\_hunkmegs to 256 will generally be OK.


* Different results compile OA without -fno-strict-aliasing 
I am using GCC7.2 and clang6.0 on ubuntu18.04.

Build OA using clang with -fno-strict-aliasing removed:
```
WARNING: light grid mismatch, l->filelen=103896, numGridPoints*8=95904
```
This is printed by renderergl2's R\_LoadLightGrid function.

    Problem solved with following line added in it.
```
ri.Printf( PRINT_WARNING, "s_worldData.lightGridBounds[i]=%d\n", s_worldData.lightGridBounds[i]);
```
    However this line do nothing but just printed the value of s_worldData.lightGridBounds[i]. I guess its a bug of clang.

    Build OA with GCC without this issue.

* E\_AddRefEntityToScene passed a refEntity which has an origin with a NaN component
* Unpure client detected. Invalid .PK3 files referenced!
* Shader rocketThrust has a stage with no image

## TODO
* merge rendergl1, rendereroa, renderer\_mydev to one module
* r\_gamma shader
* have issues with \minimize when use vulkan renderer in fullscreen. recreate the swapchain ?
* flare support
* Implement RB\_SurfaceAxis();
* Use gprof to examine the performance of the program
```
gprof openarena.x86_64 gmon.out > report.txt
```
## developing notes
```
\com_speed 1
# win10, i5-5200U, GeForce 840

frame:76190 all:  3 sv:  0 ev:  5 cl:  0 gm:  0 rf:  0 bk:  3
frame:76191 all:  3 sv:  0 ev:  5 cl:  0 gm:  0 rf:  0 bk:  3
frame:76192 all:  2 sv:  0 ev:  5 cl:  0 gm:  0 rf:  0 bk:  2
frame:76193 all:  2 sv:  0 ev:  6 cl:  0 gm:  0 rf:  0 bk:  2
frame:76194 all:  2 sv:  0 ev:  6 cl:  0 gm:  0 rf:  0 bk:  2
frame:76195 all:  3 sv:  0 ev:  6 cl:  0 gm:  0 rf:  0 bk:  3
frame:76196 all:  3 sv:  0 ev:  5 cl:  0 gm:  0 rf:  0 bk:  3
frame:76197 all:  3 sv:  0 ev:  5 cl:  0 gm:  0 rf:  0 bk:  3
frame:76198 all:  2 sv:  0 ev:  5 cl:  0 gm:  0 rf:  0 bk:  2

\timedemo 1
 
3398 frames 16.0 seconds 211.9 fps 2.0/4.7/47.0/2.1 ms

# hash function on window seems collision much
 Total 490 images, hash Table used: 366/1024

 Top 10 Collision: 34, 4, 3, 3, 3, 3, 3, 3, 3, 3

\printInstanceExtensions
 
----- Total 12 Instance Extension Supported -----
VK_EXT_debug_report
VK_EXT_display_surface_counter
VK_KHR_get_physical_device_properties2
VK_KHR_get_surface_capabilities2
VK_KHR_surface
VK_KHR_win32_surface
VK_KHR_device_group_creation
VK_KHR_external_fence_capabilities
VK_KHR_external_memory_capabilities
VK_KHR_external_semaphore_capabilities
VK_NV_external_memory_capabilities
VK_EXT_debug_utils
----- ------------------------------------- -----

# \printDeviceExtensions
--------- Total 40 Device Extension Supported ---------
 VK_KHR_swapchain
 VK_KHR_16bit_storage
 VK_KHR_bind_memory2
 VK_KHR_dedicated_allocation
 VK_KHR_descriptor_update_template
 VK_KHR_device_group
 VK_KHR_get_memory_requirements2
 VK_KHR_image_format_list
 VK_KHR_maintenance1
 VK_KHR_maintenance2
 VK_KHR_maintenance3
 VK_KHR_multiview
 VK_KHR_push_descriptor
 VK_KHR_relaxed_block_layout
 VK_KHR_sampler_mirror_clamp_to_edge
 VK_KHR_sampler_ycbcr_conversion
 VK_KHR_shader_draw_parameters
 VK_KHR_storage_buffer_storage_class
 VK_KHR_external_memory
 VK_KHR_external_memory_win32
 VK_KHR_external_semaphore
 VK_KHR_external_semaphore_win32
 VK_KHR_win32_keyed_mutex
 VK_KHR_external_fence
 VK_KHR_external_fence_win32
 VK_KHR_variable_pointers
 VK_KHX_device_group
 VK_KHX_multiview
 VK_EXT_blend_operation_advanced
 VK_EXT_depth_range_unrestricted
 VK_EXT_discard_rectangles
 VK_EXT_shader_subgroup_ballot
 VK_EXT_shader_subgroup_vote
 VK_NV_dedicated_allocation
 VK_NV_external_memory
 VK_NV_external_memory_win32
 VK_NV_glsl_shader
 VK_NV_win32_keyed_mutex
 VK_NVX_device_generated_commands
 VK_NVX_multiview_per_view_attributes
--------- ----------------------------------- ---------


total 2 Queue families:

 Queue family [0]: 16 queues,  Graphic,  Compute,  Transfer,  Sparse,  presentation supported.
 Queue family [1]: 1 queues,  Transfer,


```
