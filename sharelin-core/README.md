Welcome to the project Sharelin on Android, I am Tom, the one who wrote this project.

This code is actually used in an Android application called DroidG2, released by Toxic Lab.

Here is the current status of the project.

This project adopts code from the following projects
	Sharelin verson: 0.2.0
	http://sharelin.sourceforge.net/ 

	Github: MysticTreeGame/Boost-for-Android
	https://github.com/MysticTreeGames/Boost-for-Android
	
This project requires the following complier to complie (Or you need to have your own Boost library for Android.)
	Crystax Android NDK 4
	http://www.crystax.net/android/ndk-r4.php

The modification of Sharelin is in the form of source code, 
while the boost library is complied into libraries using the Crystax Android NDK4.

If you want to re-complie the project using other Android NDK, you need to re-compile the Boost library again.
As I don't made any modification to it, providing source code for the Boost library is a redundancy act.

Feature:
	1. Enable Android device to search files over Guntella 2 network.
	2. Enalbe Android device to download files from Gnutella 2 network.
	3. Provide an JNI interface to basic function (the JNI code files are sl.hpp and sl.cpp), such as
		a. Issue, pause, stop a search
		b. Retreive the search result
		c. Issue, pause, stop a download
	
Plan for development:
	1. File checksum are bounded to Big Endian, should have problems when using Little Endian CPU.
	2. Currently file upload are not supported
	3. Plan to upgrade the core to Sharelin version 0.2.6

This software is released under the GPL v3.0 license.

Finally, please be noticed that I don't provide any official support to this piece of software, 
I don't provide tutorial on how to use it in an Android project, and I don't bear any responsibility
for any loss caused by using this software.