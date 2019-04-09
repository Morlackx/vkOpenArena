
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

