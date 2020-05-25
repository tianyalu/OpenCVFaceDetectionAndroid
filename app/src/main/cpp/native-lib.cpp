#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <android/log.h>
#include <android/native_window_jni.h>

using namespace cv;

DetectionBasedTracker *tracker = 0;
std::vector<Rect> last_faces;
ANativeWindow *window = 0;

class CascadeDetectorAdapter : public DetectionBasedTracker::IDetector
{
public:
    CascadeDetectorAdapter(cv::Ptr<cv::CascadeClassifier> detector) :
            IDetector(),
            Detector(detector)
    {
        CV_Assert(detector);
    }

    void detect(const cv::Mat& Image, std::vector<cv::Rect>& objects)
    {
        Detector->detectMultiScale(Image, objects, scaleFactor, minNeighbours, 0, minObjSize, maxObjSize);
    }

    virtual ~CascadeDetectorAdapter()
    {

    }

private:
    CascadeDetectorAdapter();
    cv::Ptr<cv::CascadeClassifier> Detector;
};

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_opencv_facedetection_android_MainActivity_initTracker(JNIEnv *env, jobject thiz,
                                                                   jstring cascade_file_) {
    const char *cascade_file = env->GetStringUTFChars(cascade_file_, 0);
    if(tracker) {
        tracker->stop();
        delete tracker;
        tracker = 0;
    }

    //主检测适配器
    cv::Ptr<CascadeDetectorAdapter> mainDetector = makePtr<CascadeDetectorAdapter>(
            makePtr<CascadeClassifier>(cascade_file));
    //跟踪检测适配器
    cv::Ptr<CascadeDetectorAdapter> trackingDetector = makePtr<CascadeDetectorAdapter>(
            makePtr<CascadeClassifier>(cascade_file));
    //创建跟踪器
    DetectionBasedTracker::Parameters DetectorParams;
//    tracker = makePtr<DetectionBasedTracker>(mainDetector, trackingDetector, DetectorParams);
    tracker = new DetectionBasedTracker(mainDetector, trackingDetector, DetectorParams);
    //启动跟踪器
    tracker->run();

    env->ReleaseStringUTFChars(cascade_file_, cascade_file);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_sty_opencv_facedetection_android_MainActivity_getFacedData(JNIEnv *env, jobject thiz,
                                                                    jbyteArray data_, jint w, jint h,
                                                                    jint camera_id) {
    jbyte *data = env->GetByteArrayElements(data_, 0);
    //1、高 2、宽
    Mat src(h + h / 2, w, CV_8UC1, data);
    //将 nv21的yuv数据转成了rgba
    cvtColor(src, src, COLOR_YUV2RGBA_NV21);

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

    bool needBreak = false;
    for (Rect face : faces) {
        //画矩形
        if (!last_faces.empty()) {
            //遍历前面检测的人脸框是否与当前检测到的重叠
            for (Rect last_face : last_faces) {
                if (!last_face.empty()) {
                    //IOU：两个矩形的交并比， 两个矩形的交集除以两个矩形的并集
                    double iou = (last_face & face).area() * 1.0 / (last_face | face).area();
                    __android_log_print(ANDROID_LOG_ERROR, "Face", "iou: %lf", iou);
                    if (iou >= 0.8) {
                        rectangle(src, last_face, Scalar(0, 255, 0));
                        needBreak = true;
                        break;
                    }
                }
            }
            if(needBreak) {
                break;
            }
        }
        last_faces.push_back(face);
        rectangle(src, face, Scalar(0, 255, 0));
    }

    //显示
//    if (window) {
//        //设置windows的属性
//        // 因为旋转了 所以宽、高需要交换
//        //这里使用 cols 和rows 代表 宽、高 就不用关心上面是否旋转了
//        ANativeWindow_setBuffersGeometry(window, src.cols, src.rows, WINDOW_FORMAT_RGBA_8888);
//        ANativeWindow_Buffer buffer;
//        do {
//            //lock失败 直接brek出去
//            if (ANativeWindow_lock(window, &buffer, 0)) {
//                ANativeWindow_release(window);
//                window = 0;
//                break;
//            }
//            //src.data ： rgba的数据
//            //把src.data 拷贝到 buffer.bits 里去
//            // 一行一行的拷贝
//            memcpy(buffer.bits, src.data, buffer.stride * buffer.height * 4);
//            //提交刷新
//            ANativeWindow_unlockAndPost(window);
//        } while (0);
//    }

    gray.release();
    env->ReleaseByteArrayElements(data_, data, 0);

//    unsigned char* dataArray = new unsigned char[src.rows * src.cols];
//    if(src.isContinuous()) {
//        dataArray = src.data;
//    }

    jbyteArray  jbyteArray1;
    return jbyteArray1;
//    return (jbyteArray) dataArray;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_opencv_facedetection_android_MainActivity_releaseTracker(JNIEnv *env, jobject thiz) {
    if(tracker) {
        tracker->stop();
        delete tracker;
        tracker = 0;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_opencv_facedetection_android_CameraHelper_setSurface(JNIEnv *env, jobject thiz,
                                                                  jobject surface) {
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);
}

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

    bool needBreak = false;
    for (Rect face : faces) {
        //画矩形
//        if (!last_faces.empty()) {
//            //遍历前面检测的人脸框是否与当前检测到的重叠
//            for (Rect last_face : last_faces) {
//                if (!last_face.empty()) {
//                    //IOU：两个矩形的交并比， 两个矩形的交集除以两个矩形的并集
//                    double iou = (last_face & face).area() * 1.0 / (last_face | face).area();
//                    __android_log_print(ANDROID_LOG_ERROR, "Face", "iou: %lf", iou);
//                    if (iou >= 0.8) {
//                        rectangle(src, last_face, Scalar(0, 255, 0));
//                        needBreak = true;
//                        break;
//                    }
//                }
//            }
//            if(needBreak) {
//                break;
//            }
//        }
//        last_faces.push_back(face);
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