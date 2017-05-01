#ifndef calibration
#define calibration

#include <libfreenect2/libfreenect2.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cstdlib>

class Box {
  cv::Rect bounding_rect;

  public:
    Box();
    int getLeft();
    int getRight();
    int getTop();
    int getBottom();
    cv::Rect getBox();
    void findBox(cv::Mat img, bool showImg);
};

class RedBox {
  cv::Rect bounding_rect;

  public:
    RedBox();
    int getLeft();
    int getRight();
    int getTop();
    int getBottom();
    void findBox(cv::Mat img, bool showImg);
};

#endif
