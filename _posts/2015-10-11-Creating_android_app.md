---
layout: post
title: Droid-G2 development guide 3/3 
header: Droid-G2 development guide 3/3 
---


Welcome to the third and final step of compiling the Droid-G2.

The goal of this step is to create Droid-G2 application, create the APK file, deploy and run Droid-G2 on the Android platform

Complete the following steps to create the APK file:

* Place the libsharelin.so to the folder /libs/armeabi
* Declare the native functions. Example: <https://github.com/Toxic-Lab/DroidG2/blob/master/DroidG2-app/src/org/toxiclab/droidg2/main.java>
{% highlight java %}
// A list of native method declaration 
public native void start_sharelin(String root);
public native void createSearch(String dn, String min, String max, String media, String ext);
public native toxiclab_search[] showSearch();
public native toxiclab_searchResult[] showSearchResult(int id, String sortBy, boolean forward);
public native toxiclab_hub[] showHubs();
public native toxiclab_hub[] showHandshake();
public native toxiclab_downloadResult showDownload(int id);
public native toxiclab_download[] showAllDownload(String sortBy, boolean forward);
public native boolean createDownload(int searchID, int resultID);
public native toxiclab_incoming[] showIncoming();
public native void pauseDownload(int id);
public native void resumeDownload(int id);
public native void deleteDownload(int id);
public native void pauseSearch(int id);
public native void deleteSearch(int id);
public native void resumeSearch(int id);
public native void killCore();
{% endhighlight %}
* Create a static block to load the native library
{% highlight java %}
static{
    System.loadLibrary("sharelin");
}
{% endhighlight %}
* Implement the rest of the user interface
* Compile the whole project and generate the APK.

Great, here is all you have to know about compiling the Droid-G2 Android app. If you want to know more about a particular step or you get stuck, you can drop an email to <kongutoxiclab@gmail.com>, and we will try our best to spare some time to answer your questions.