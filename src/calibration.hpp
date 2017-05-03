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
    cv::Rect getBox();
    void findBox(cv::Mat img, bool showImg);
};

class RedBox {
  cv::Rect bounding_rect;
  cv::Mat box;

  int lWall;
  int tWall;
  int rWall;
  int bWall;

  private:
    void findBox(cv::Mat img, bool showImg);

  public:
    RedBox();
    void draw();
    void drawRedSquare(int size);
    bool findWalls(cv::Mat img, bool showImg);
};

int getLeft(cv::Rect bounding_rect);
int getRight(cv::Rect bounding_rect);
int getTop(cv::Rect bounding_rect);
int getBottom(cv::Rect bounding_rect);

#endif
