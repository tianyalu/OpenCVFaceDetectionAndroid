#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include "MyOpenCVHelper.h"

DetectionBasedTracker *tracker = 0;

ANativeWindow *window = nullptr;
MyOpenCVHelper *myOpenCvHelper = nullptr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //静态初始化

void renderFrame(uint8_t *rgba, int width, int height);

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_opencv_facedetection_android_MainActivity_getFaceData(JNIEnv *env, jobject thiz,
                                                                   jbyteArray data_, jint w,
                                                                   jint h, jint camera_id) {
    jbyte *data = env->GetByteArrayElements(data_, 0);
    //1、高 2、宽
    Mat src(h + h / 2, w, CV_8UC1, data);
    Mat mat_yuv = Mat(h + h / 2, w, CV_8UC1, data);
    Mat mat_rgb = Mat(h, w, CV_8UC4);
//    Mat src = Mat(h, w, CV_8UC4);
    //将 nv21的yuv数据转成了rgba
    cvtColor(src, src, COLOR_YUV2RGBA_NV21);
//    cvtColor(mat_yuv, src, COLOR_YUV2RGBA_NV21);

    if (camera_id == 1) {
        //前置摄像头，需要逆时针旋转90度
        rotate(src, src, ROTATE_90_COUNTERCLOCKWISE);
        //水平翻转 镜像
        flip(src, src, 1);
    } else {
        //顺时针旋转90度
        rotate(src, src, ROTATE_90_CLOCKWISE);
    }

    Mat gray;
    //灰色
    cvtColor(src, gray, COLOR_RGBA2GRAY);
    //增强对比度 (直方图均衡)
    equalizeHist(gray, gray);
    std::vector<Rect> faces;
    //定位人脸 N个
    tracker->process(gray);
    tracker->getObjects(faces);

    __android_log_print(ANDROID_LOG_ERROR, "Face", "faceSize: %d", faces.size());

    for (Rect face : faces) {
        //画矩形
        rectangle(src, face, Scalar(0, 255, 0));
    }

    //显示
    if (window) {
        //设置windows的属性
        // 因为旋转了 所以宽、高需要交换
        //这里使用 cols 和rows 代表 宽、高 就不用关心上面是否旋转了
        ANativeWindow_setBuffersGeometry(window, src.cols, src.rows, WINDOW_FORMAT_RGBA_8888);
        ANativeWindow_Buffer buffer;
        do {
            //lock失败 直接brek出去
            if (ANativeWindow_lock(window, &buffer, 0)) {
                ANativeWindow_release(window);
                window = 0;
                break;
            }
            //src.data ： rgba的数据
            //把src.data 拷贝到 buffer.bits 里去
            // 一行一行的拷贝
            memcpy(buffer.bits, src.data, buffer.stride * buffer.height * 4);
            //提交刷新
            ANativeWindow_unlockAndPost(window);
        } while (0);
    }

    gray.release();
    env->ReleaseByteArrayElements(data_, data, 0);
}

//初始化，人脸模型路径
extern "C"
JNIEXPORT void JNICALL
Java_com_sty_opencv_facedetection_android_MyOpenCVHelper_initNative(JNIEnv *env, jobject thiz,
                                                                    jstring face_model_path_) {
    const char *path = env->GetStringUTFChars(face_model_path_, 0);

    if(!myOpenCvHelper) {
        myOpenCvHelper = new MyOpenCVHelper(path);
        myOpenCvHelper->initDetectorTracker();
    }

    env->ReleaseStringUTFChars(face_model_path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_opencv_facedetection_android_MyOpenCVHelper_setSurfaceNative(JNIEnv *env, jobject thiz,
                                                                          jobject surface) {
    pthread_mutex_lock(&mutex);
    //先移除之前的window
    if(window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
    //创建新的window
    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_opencv_facedetection_android_MyOpenCVHelper_renderFrameNative(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jbyteArray bytes,
                                                                           jint width,
                                                                           jint height) {
    jbyte *yuv = env->GetByteArrayElements(bytes, 0);

    //NV21转rgb（网上找的一个算法）
    uint8_t *rgb = myOpenCvHelper->nv21ToRgb(reinterpret_cast<uint8_t *>(yuv), width, height);
    //rgb转mat
    Mat frame = myOpenCvHelper->rgb2Mat(rgb, width, height);
    myOpenCvHelper->dynamicFaceDetect(frame);
    //mat转rgb
    rgb = myOpenCvHelper->mat2rgb(frame, 0);
    //rgb转rgba
    uint8_t *rgba = myOpenCvHelper->rgb2rgba(rgb, width, height, 50);
    //渲染
    renderFrame(rgba, width, height);

    env->ReleaseByteArrayElements(bytes, yuv, 0);
}

void renderFrame(uint8_t *rgba, int width, int height) {
    pthread_mutex_lock(&mutex);
    if(!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    //初始化窗口属性
    //注意：这里的宽高是指 图像的宽高
    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;
    if(ANativeWindow_lock(window, &buffer, nullptr)) {
        ANativeWindow_release(window);
        window = nullptr;
        return;
    }
    uint8_t *windowData = static_cast<uint8_t *>(buffer.bits);

    //原始数据中一行的rgba图像的字节数
    int lineSize = width * 4;
    //窗口中一行图像的字节数
    int windowDataLineSize = buffer.stride * 4;
    //内存拷贝，逐行拷贝
    for (int i = 0; i < buffer.height; ++i) {
        memcpy(windowData + i * windowDataLineSize, rgba + i * lineSize, windowDataLineSize);
    }

    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}
