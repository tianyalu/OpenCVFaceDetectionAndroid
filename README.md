# OpenCV 人脸检测 Android 实现

[TOC]

## 一、实现步骤

### 1.1 下载OpenCV

去 [`OpenCV`官网](https://opencv.org/releases/) 下载合适的 **`Android`** 版本并解压，这里以`OpenCV-4.1.2`为例。  

### 1.2 `Android Studio` 集成 `OpenCV`
#### 1.2.1 新建带有`C++`库的`Android`项目，并拷贝库文件
新建带有`C++`库的`Android`项目，在`app/src/main/`目录下新建`jniLibs`目录，并将`OpenCV-android-sdk/sdk/native/libs/`下的`armeabi-v7a`目录拷贝到`jniLibs`目录中。  在`app`下的`build.gradle`中的`defaultConfig`中添加：  

```groovy
ndk {
    abiFilters 'armeabi-v7a'
}
```

#### 1.2.2 拷贝头文件

将`OpenCV-android-sdk/sdk/native/jni/`下的`include`目录拷贝到`app/src/main/cpp/`目录下,然后修改该目录下的`CMakeLists.txt`文件：  

```cmake
cmake_minimum_required(VERSION 3.4.1)

#指定头文件查找路径
include_directories(include)

#指定库的查找路径
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI}")

add_library( # Sets the name of the library.
        native-lib
        SHARED
        native-lib.cpp)

target_link_libraries( # Specifies the target library.
        native-lib
        opencv_java4
        log)
```

#### 1.2.3 拷贝人脸识别模型文件

将`OpenCV-android-sdk/samples/face-detection/res/`下的`raw`目录拷贝到我们自己项目中的`app/src/main/res/`目录下，这个是人脸识别的模型对比文件。

#### 1.2.4 拷贝`Java`文件和布局文件

在项目中建立名为`org.opencv.samples.facedetect`的包，然后将``OpenCV-android-sdk/samples/face-detection/src/org/opencv/samples/facedetect/`目录下的`DetectionBasedTracker.java`和`FdActivity.java`文件复制到该包下，然后将`FdActivity`加入到`AndroidManifest.xml`中；将`OpenCV-android-sdk/samples/face-detection/res/layout/`下的`face_detect_surface_view.xml`拷贝到`app/src/main/res/layout/`目录下。

#### 1.2.5 拷贝`C++`文件并修改`CMakeLists.txt`文件

将`OpenCV-android-sdk/samples/face-detection/jni/`目录下的`DetectionBasedTracker_jni.cpp`和`DetectionBasedTracker_jni.h`文件复制到`app/src/main/cpp/`目录下，然后修改`CMakeLists.txt`文件：  

```cmake
add_library( # Sets the name of the library.
        native-lib
        SHARED
        DetectionBasedTracker_jni.cpp)  #这里之前为 native-lib.cpp，现修改为 DetectionBasedTracker_jni.cpp
```

修改`FdActivity.java`中的`BaseLoaderCallback`监听中的加载库：  

```java
System.loadLibrary("native-lib");
```

#### 1.2.6 添加权限

`AndroidManifest.xml`文件中添加权限：  

```xml
<uses-permission android:name="android.permission.CAMERA"/>
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"/>
<uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE"/>

<uses-feature android:name="android.hardware.camera" android:required="false"/>
<uses-feature android:name="android.hardware.camera.autofocus" android:required="false"/>
<uses-feature android:name="android.hardware.camera.front" android:required="false"/>
<uses-feature android:name="android.hardware.camera.front.autofocus" android:required="false"/>
```

#### 1.2.7 引入`OpenCV Module`作为库使用

`File->New->Import Module...`选择将`OpenCV-android-sdk/sdk/`下的`java`目录导入项目，然后修改该`Module`中`src`目录下的`build.gradle`文件：  

```groovy
//apply plugin: 'com.android.application'
apply plugin: 'com.android.library'

android {
    compileSdkVersion 28
    buildToolsVersion "28.0.3"

    defaultConfig {
//        applicationId "org.opencv"
    }

    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android.txt'), 'proguard-rules.txt'
        }
    }
}
```

然后在“项目结构视图”中将该库作用`app`的依赖。

## 二、采坑  

### 2.1 `DetectionBasedTracker_jni.cpp`文件警告

警告信息如下：  

```bash
To make sure the C++ compiler uses unmodified names when calling functions in your C code,list your C functions in your C++ code usinng extern "C".
```

解决方案：

如警告所建议，在方法前添加`extern "C"`。  

### 2.3 `Package not found`

运行显示提示框：  

```bash
OpenCV Manager package was not found! Try to install it?
```

原因：动态库没有真正生成。  

解决方案：  修改`app`目录下的`gradle`文件

```groovy
defaultConfig {
  applicationId "com.sty.opencv.facedetection.android"
  minSdkVersion 19
  targetSdkVersion 28
  versionCode 1
  versionName "1.0"
  testInstrumentationRunner "android.support.test.runner.AndroidJUnitRunner"
  externalNativeBuild {
    cmake {
      //OpenCV 4.x+ requires enabled C++11 support
      cppFlags "-std=c++11"
      abiFilters "armeabi-v7a"
      arguments "-DANDROID_STL=c++_shared"
    }
  }
  ndk {
    abiFilters 'armeabi-v7a'
  }
}
```

