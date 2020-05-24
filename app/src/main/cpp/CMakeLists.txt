cmake_minimum_required(VERSION 3.4.1)

#指定头文件查找路径
include_directories(include)

#指定库的查找路径
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI}")

add_library( # Sets the name of the library.
        native-lib
        SHARED
        native-lib.cpp    # 这里自己实现检测
        DetectionBasedTracker_jni.cpp)  #这里之前为 native-lib.cpp，现修改为 DetectionBasedTracker_jni.cpp

target_link_libraries( # Specifies the target library.
        native-lib
        opencv_java4
        android
        log)