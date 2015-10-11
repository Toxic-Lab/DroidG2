---
layout: post
title: Droid-G2 development guide 2/3 
header: Droid-G2 development guide 2/3 
---

Welcome to the second step of compiling the Droid-G2.

The goal of this step is to compile the Sharelin library, link all the boost library object files, and create a single file called libsharelin.so.

* Download the source of sharelin from the sourceforge. <http://sourceforge.net/projects/sharelin/>
* Check that CrystalX NDK is in-place.
* Copy the boost static library files and header file to a subfolder under project directory (In this project, it is copied to /boost/lib and /boost/include respectively).
* Create the Android.MK file. <https://github.com/Toxic-Lab/DroidG2/blob/master/sharelin-core/jni/Android.mk>
* Compile the code using CrystalX Android NDK first. You will discovered some compilation error because of missing CONSTANT and functions in Android Linux header.
    * These errors can be fixed by simply defining the CONSTANT and use the overload methods of the missing function.
* You also needed to define JNI interface for interact with your Android application.
    * Define the java interface 
    * Follow the JNI tutorial to generate the .hpp file <http://docs.oracle.com/javase/7/docs/technotes/tools/solaris/javah.html>
    * Create .cpp file and implement the function in the header file
    * Example: 
        * sl.cpp: <https://github.com/Toxic-Lab/DroidG2/blob/master/sharelin-core/jni/src/sl.cpp>
        * sl.hpp: <https://github.com/Toxic-Lab/DroidG2/blob/master/sharelin-core/jni/src/sl.hpp>
* Use make to compile the project
* Now we have the libsharelin.so file and have native C functions talking with Java JVM.

In the next post, we are going to present how to compile the Droid-G2 android application.