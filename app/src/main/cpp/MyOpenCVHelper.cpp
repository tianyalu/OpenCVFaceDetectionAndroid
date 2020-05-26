//
// Created by 天涯路 on 2020-05-26.
//

#include "MyOpenCVHelper.h"

MyOpenCVHelper::MyOpenCVHelper(const char *faceModelPath){
    this->faceModelPath = new char[strlen(faceModelPath) + 1];
    strcpy(this->faceModelPath, faceModelPath);
}

MyOpenCVHelper::~MyOpenCVHelper() {

}

//初始化追踪器
void MyOpenCVHelper::initDetectorTracker() {
    //主检测适配器
    cv::Ptr<CascadeDetectorAdapter> mainDetector = makePtr<CascadeDetectorAdapter>(
            makePtr<CascadeClassifier>(faceModelPath));
    //跟踪检测适配器
    cv::Ptr<CascadeDetectorAdapter> trackingDetector = makePtr<CascadeDetectorAdapter>(
            makePtr<CascadeClassifier>(faceModelPath));
    //创建跟踪器
    DetectionBasedTracker::Parameters DetectorParams;
    detectorTracker = makePtr<DetectionBasedTracker>(mainDetector, trackingDetector, DetectorParams);
    //启动跟踪器
    detectorTracker->run();
    tracking = 1;
}

//动态人脸检测，这种方式适用于视频检测
//动态检测需要用到适配器
void MyOpenCVHelper::dynamicFaceDetect(Mat &frame) {
    if(!tracking) {
        LOGE("tracker is not running");
        return;
    }
    //灰度图
    Mat gray;
    if(frame.empty()) {
        LOGE("frame is empty!");
        return;
    }
    //转灰度图
    cvtColor(frame, gray, COLOR_RGB2GRAY);
    //直方图均衡化，增强对比度
    equalizeHist(gray, gray);
    //检测向量
    std::vector<Rect> faces;
    //人脸检测处理
    detectorTracker->process(gray);
    //获取检测结果
    detectorTracker->getObjects(faces);
    //画人脸框
    for (int i = 0; i < faces.size(); ++i) {
        Rect face = faces[i];
        //在原始图上画一个绿色的框；
        rectangle(frame, face, Scalar(0, 255, 0));
    }
}

/**
 * NV21转RGB,这是根据NV21数据结构和存储方式自己写的算法
 * @param nv21
 * @param width
 * @param height
 * @return
 */
uint8_t *MyOpenCVHelper::nv21ToRgb(uint8_t *nv21,int width,int height) {
    //定义一个二维数组存储像素，与openCv的Mat矩阵式是对应的，
    //Mat(高，宽), CV_8UC3:三个通道，每个通道8位
    uint8_t rgb[height][width * 3];
    uint8_t r, g, b;                   //r g b分量
    int y, v, u;                       //y u v分量
    int p_y, p_v, p_u;                 //y u v分量的位置索引
    int frameSize = width * height;

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            //y u v分量位置索引算法（自己：效率低，但容易理解）
            //当前像素y分量对应的uv分量的索引，有些复杂，可以一步一步验证
            int index = frameSize + h / 2 * width + w / 2 * 2;
            //这里使用的是NV21格式，计算方式需要结合NV21的排列
            p_y = h * width + w;
            p_v = index;
            p_u = index + 1;

            y = nv21[p_y];
            v = nv21[p_v];
            u = nv21[p_u];

//            //公式一（老师给出）
//            double R = y + 1.402 * (v - 128);
//            double G = y - 0.34413 * (u - 128) - 0.71414 * (v - 128);
//            double B = y + 1.772 * (u - 128);

//            //公式二（网上）
//            double R = (uint8_t)(y + 1.4075 * (v - 128));
//            double G = (uint8_t)((y - 0.3455 * (u - 128) - 0.7169 * (v - 128)));
//            double B = (uint8_t)((y + 1.779 * (u - 128)));

//            //公式三（网上）
//            double R = ((y-16) * 1.164 + (v-128) * 1.596);
//            double G = ((y-16) * 1.164 - (u-128) * 0.392 - (v-128) * 0.813);
//            double B = ((y-16) * 1.164 + (u-128) * 2.017);

//            //公式四（网上）
//            double R = (1.164*y + 1.596 * v - 222.9);
//            double G = (1.164*y - 0.392 * u - 0.823 * v + 135.6);
//            double B = (1.164*y + 2.017 * u- 276.8);

//            //公式五（网上）
//            int R = (298*y + 411 * v - 57344)>>8;
//            int G = (298*y - 101* u - 211* v+ 34739)>>8;
//            int B = (298*y + 519* u - 71117)>>8;

            //公式六（网上）
            int R = y + ((360 * (v - 128))>>8) ;
            int G = y - ((( 88 * (u - 128)  + 184 * (v - 128)))>>8) ;
            int B = y +((455 * (u - 128))>>8) ;

            r = (uint8_t) (R > 255 ? 255 : (R < 0 ? 0 : R));
            g = (uint8_t) (G > 255 ? 255 : (G < 0 ? 0 : G));
            b = (uint8_t) (B > 255 ? 255 : (B < 0 ? 0 : B));

            rgb[h][w * 3 + 0] = r;
            rgb[h][w * 3 + 1] = g;;
            rgb[h][w * 3 + 2] = b;
        }
    }
    return *rgb;
}


/**
 * rgb转灰度，这里为了学习矩阵的操作都是直接对矩阵进行遍历赋值的，
 * openCv应该提供了相应的api，以后再研究
 * @param rgb
 * @param width
 * @param height
 * @return
 */
uint8_t *MyOpenCVHelper::transformGray(uint8_t *rgb, int width, int height) {
    //rgb指针转成openCv的Mat（3通道）
    Mat color = rgb2Mat(rgb,width,height);
    Mat gray;
    //将原Mat转成灰度的Mat（1通道）
    cvtColor(color,gray,COLOR_RGB2GRAY);
    //再将灰度的Mat还原成rgb格式的Mat（3通道）
    cvtColor(gray,color,COLOR_GRAY2RGB);
    //最后将rgb格式的Mat转成rgb指针
    return mat2rgb(color,0);
}

Mat MyOpenCVHelper::rgb2Mat(uint8_t *rgb, int width, int height) {
    Mat mat;
    mat.create(height,width,CV_8UC3);
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            int index = h * width * 3;
            mat.at<Vec3b>(h,w)[0] = rgb[index + w * 3 + 0];
            mat.at<Vec3b>(h,w)[1] = rgb[index + w * 3 + 1];
            mat.at<Vec3b>(h,w)[2] = rgb[index + w * 3 + 2];
        }
    }
    return mat;
}

/**
 * Mat转rgb，只支持CV_8UC3
 * @param mat
 * @param colorType 颜色的类型，0：rgb，1：bgr
 * @return
 */
uint8_t* MyOpenCVHelper::mat2rgb(Mat mat, int colorType){
    if(mat.type() != CV_8UC3){
        throw "此方法只支持CV_8UC3类型的转换！";
    }
    int height = mat.rows;
    int width = mat.cols;
    uint8_t rgbArray[height][width * 3];
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            if(colorType == 0){
                rgbArray[h][w * 3 + 0] = mat.at<Vec3b>(h,w)[0];
                rgbArray[h][w * 3 + 1] = mat.at<Vec3b>(h,w)[1];
                rgbArray[h][w * 3 + 2] = mat.at<Vec3b>(h,w)[2];
            } else if(colorType == 1){
                rgbArray[h][w * 3 + 0] = mat.at<Vec3b>(h,w)[2];
                rgbArray[h][w * 3 + 1] = mat.at<Vec3b>(h,w)[1];
                rgbArray[h][w * 3 + 2] = mat.at<Vec3b>(h,w)[0];
            }
        }
    }
    return *rgbArray;
}

uint8_t *MyOpenCVHelper::rgb2rgba(uint8_t *data, int width, int height, uint8_t alpha) {
    uint8_t rgba[height][width * 4];
    //data一行的字节数，*3：有三个通道
    int lineSize = width * 3;
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            rgba[h][w * 4 + 0] = data[h * lineSize + w * 3 + 0];
            rgba[h][w * 4 + 1] = data[h * lineSize + w * 3 + 1];
            rgba[h][w * 4 + 2] = data[h * lineSize + w * 3 + 2];
            rgba[h][w * 4 + 3] = alpha;
        }
    }
    return *rgba;
}

/**
 * NV21转RGB,这是老师给出的一种算法,有点小问题：应该做一些越界（0-288）判断
 * @param nv21
 * @param width
 * @param height
 * @return
 */
//uint8_t *MyOpenCVHelper::nv21ToRgb(uint8_t *nv21,int width,int height) {
//    //定义一个二维数组存储像素，与openCv的Mat矩阵式是对应的，
//    //Mat(高，宽), CV_8UC3:三个通道，每个通道8位
//    uint8_t rgb[height*width * 3];
//    int index = 0;
//    uint8_t *yBase = nv21;
//    uint8_t *uBase = &nv21[width*height];
//    int y, v, u;       //y u v分量
//
//    for (int h = 0; h < height; h++) {
//        for (int w = 0; w < width; w++) {
//
//            y = yBase[w + h * width];
//            v = uBase[h / 2 * width + w / 2 * 2];
//            u = uBase[h / 2 * width + w / 2 * 2 + 1];
//
//            rgb[index++] = y + 1.402 * (v - 128);
//            rgb[index++] = y - 0.34413 * (u - 128) - 0.71414 * (v - 128);
//            rgb[index++] = y + 1.772 * (u - 128);
//        }
//    }
//    return rgb;
//}

/**
 * NV21转RGB,这是网上找的一种算法，可以实现，但是有待研究
 * @param nv21
 * @param width
 * @param height
 * @return
 */
//uint8_t *MyOpenCVHelper::nv21ToRgb(int8_t *nv21,int width,int height) {
//    //输出归一化（0-255）的rgb数据
//    uint8_t rgb[height][width * 3];
//    //图像大小
//    const int frameSize = width * height;
//    //归一化因子
//    const int normalized = 255;
//    //未归一化r g b分量
//    int r,g,b;
//    //归一化r g b分量
//    uint8_t R,G,B;
//    //y u v 分量
//    int y,u,v;
//    //uv位置索引
//    int p_uv;
//    for (int j = 0, yp = 0; j < height; j++) {
//        p_uv = frameSize + (j >> 1) * width;
//        u = 0;
//        v = 0;
//        for (int i = 0; i < width; i++, yp++) {
//            y = (0xff & ((int) nv21[yp])) - 16;
//            if (y < 0)
//                y = 0;
//            if ((i & 1) == 0) {
//                v = (0xff & nv21[p_uv++]) - 128;
//                u = (0xff & nv21[p_uv++]) - 128;
//            }
//
//            int y1192 = 1192 * y;
//            r = (y1192 + 1634 * v);
//            g = (y1192 - 833 * v - 400 * u);
//            b = (y1192 + 2066 * u);
//
//            if (r < 0){
//                r = 0;
//            }else if (r > 262143){
//                r = 262143;
//            }
//
//            if (g < 0){
//                g = 0;
//            }else if (g > 262143){
//                g = 262143;
//            }
//
//            if (b < 0){
//                b = 0;
//            }else if (b > 262143){
//                b = 262143;
//            }
//
//            //归一化为（0-255）
//            R = (uint8_t)(r * normalized / 262143);
//            G = (uint8_t)(g * normalized / 262143);
//            B = (uint8_t)(b * normalized / 262143);
//
//            rgb[j][i * 3 + 0] = R;
//            rgb[j][i * 3 + 1] = G;;
//            rgb[j][i * 3 + 2] = B;
//        }
//    }
//    return *rgb;
//}

/**
 * NV21转RGB,网上的另一种算法，颜色不是很对，可能和整数运算有关
 * @param nv21
 * @param width
 * @param height
 * @return
 */
//uint8_t *MyOpenCVHelper::nv21ToRgb(uint8_t *nv21,int width,int height) {
//    //定义一个二维数组存储像素，与openCv的Mat矩阵式是对应的，
//    //Mat(高，宽), CV_8UC3:三个通道，每个通道8位
//    uint8_t rgb[height][width * 3];
//    uint8_t r, g, b;                   //r g b分量
//    int y, v, u;                       //y u v分量
//    int p_y, p_v, p_u;                 //y u v分量的位置索引
//    int frameSize = width * height;
//
//    for (int h = 0; h < height; h++) {
//        for (int w = 0; w < width; w++) {
//
//            //y u v分量位置索引算法：效率高，但不易理解
//            p_y = h * width + w;
//            p_u = frameSize + (h >> 1) * width + (w& ~1) + 1;
//            p_v = frameSize + (h >> 1) * width + (w& ~1) + 0;
//
//            y = nv21[p_y];
//            v = nv21[p_v];
//            u = nv21[p_u];
//
//            //转换
//            y = y - 16 > 0 ? y - 16 : 0;
//            u = u - 128 > 0 ? u - 128 : 0;
//            v = v - 128 > 0 ? v - 128 : 0;
//
//            int R = (116 * y + 160 * v) / 100;
//            int G = (116 * y - 81 * v - 39 * u) / 100;
//            int B = (116 * y + 202 * u) / 100;
//
//            r = (uint8_t) (R > 255 ? 255 : (R < 0 ? 0 : R));
//            g = (uint8_t) (G > 255 ? 255 : (G < 0 ? 0 : G));
//            b = (uint8_t) (B > 255 ? 255 : (B < 0 ? 0 : B));
//
//            rgb[h][w * 3 + 0] = r;
//            rgb[h][w * 3 + 1] = g;;
//            rgb[h][w * 3 + 2] = b;
//        }
//    }
//    return *rgb;
//}