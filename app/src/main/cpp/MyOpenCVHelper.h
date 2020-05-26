//
// Created by 天涯路 on 2020-05-26.
//

#ifndef OPENCVFACEDETECTIONANDROID_MYOPENCVHELPER_H
#define OPENCVFACEDETECTIONANDROID_MYOPENCVHELPER_H

#include "CascadeDetectorAdapter.h"
#include "AndroidLog.h"


class MyOpenCVHelper {
public:
    MyOpenCVHelper(const char *faceModelPath);

    ~MyOpenCVHelper();

    void initDetectorTracker();
    void dynamicFaceDetect(Mat &frame);                                      //动态人脸检测

    uint8_t* transformGray(uint8_t *rgb,int width,int height);              //rgb转灰度图
    Mat rgb2Mat(uint8_t *rgb,int width,int height);                         //rgb转Mat
    uint8_t*  mat2rgb(Mat mat, int colorType);                              //Mat转rgb
    uint8_t* nv21ToRgb(uint8_t *nv21,int width,int height);                  //NV21转rgb
    uint8_t* rgb2rgba(uint8_t *data, int width, int height, uint8_t alpha); //rgb转rgba


private:
    char *faceModelPath = nullptr;                        //人脸模型路径
    Ptr<DetectionBasedTracker> detectorTracker = nullptr; //追踪器
    int tracking = 0;                                     //追踪器运行状态
};


#endif //OPENCVFACEDETECTIONANDROID_MYOPENCVHELPER_H
