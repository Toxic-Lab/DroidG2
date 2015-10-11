---
layout: post
title: Droid-G2 development guide 1/3 
header: Droid-G2 development guide 1/3 
---

Creating the Droid G2 android application requires 3 steps:

* Compile the boost library into static libraries using Android NDK
    * Please beware you are compile against the ARM platform. check the result with readelf command.
* Compile the Sharelin library and linked the boost static libraries
    * The output is the libsharelin.so, which is a dynamic library which is loaded by JVM at runtime.
* Create a Android application, load the libsharelin.so, and start the Gnutella2 client on android through Java native interface

I will go through each step with a post.
In this post, I am going to reproduce my way to compile the boost library on Android.

Environment: Ubuntu, CrystalX NDK, CMake.

Steps:
* Download Boost library version 1.45 
* Download Crystal Android NDK 4 <https://www.crystax.net/android/ndk/4> 
* Apply patches to the Boost library 
* Follow the instruction from the <https://github.com/MysticTreeGames/Boost-for-Android/blob/master/README.md>

The following boost library in static library format (.a) is needed to compile the sharelin.
    * Filesyste library
    * Multi-thread library
    * System library

Check the above library using readelf command before proceeding to the next step. This really save you a lot of debugging time.

Congratulations, you can proceed to the next step.
