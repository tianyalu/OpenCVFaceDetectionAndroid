//
// Created by 天涯路 on 2020-05-26.
//

#ifndef OPENCVFACEDETECTIONANDROID_CASCADEDETECTORADAPTER_H
#define OPENCVFACEDETECTIONANDROID_CASCADEDETECTORADAPTER_H

#include <opencv2/opencv.hpp>

using namespace cv;

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
#endif //OPENCVFACEDETECTIONANDROID_CASCADEDETECTORADAPTER_H
