#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <cstdio>

#include "calibration.hpp"

using namespace std;
using namespace cv;

Box::Box() {}

void Box::findBox(Mat img, bool showImg) {
  Mat hsv_image, dimg, bgr, R, G, B;

  // Image comes in as BGRA
  extractChannel(img, B, 0);
  extractChannel(img, G, 1);
  extractChannel(img, R, 2);

  std::vector<cv::Mat> array_to_merge;

  array_to_merge.push_back(B);
  array_to_merge.push_back(G);
  array_to_merge.push_back(R);

  // Create a bgr mat
  merge(array_to_merge, bgr);

  // Turn into hsv
  cvtColor(bgr, hsv_image, COLOR_BGR2HSV_FULL);

  // Calibrate for color
  inRange(hsv_image, Scalar(20, 50, 75), Scalar(40, 100, 230), dimg);
  GaussianBlur(dimg, dimg, Size(9, 9), 2, 2);

  // imshow("dank", dimg);
  // waitKey();

  vector<vector<Point> > contours;
  findContours(dimg, contours, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

  int largest_contour_index = 0;
  double largest_area = 0;

  for(size_t i = 0; i < contours.size(); i++) { // iterate through each contour.
    double a = contourArea(contours[i], false);

    if(a>largest_area){
      largest_area=a;
      largest_contour_index=i;                  //Store the index of largest contour
      bounding_rect=boundingRect(contours[i]);  // Find the bounding rectangle for biggest contour
    }
  }

  if (showImg) {
    drawContours(bgr, contours, largest_contour_index, Scalar(255, 0, 0), 2);
    rectangle(bgr, bounding_rect, Scalar(0, 255, 0), 2, 8, 0);
    imshow("depthBox", bgr);
  }
}

Rect Box::getBox() {
  return bounding_rect;
}

int Box::getLeft() {
  return bounding_rect.x;
}

int Box::getRight() {
  return bounding_rect.x + bounding_rect.width;
}

int Box::getTop() {
  return bounding_rect.y;
}

int Box::getBottom() {
  return bounding_rect.y + bounding_rect.height;
}

RedBox::RedBox() {}

void RedBox::findBox(Mat img, bool showImg) {
  Mat hsv_image, bgr, R, G, B;
  Mat lower_red_hue_range, upper_red_hue_range, red_hue_image;

  // Image comes in as BGRA
  extractChannel(img, B, 0);
  extractChannel(img, G, 1);
  extractChannel(img, R, 2);

  std::vector<cv::Mat> array_to_merge;

  array_to_merge.push_back(B);
  array_to_merge.push_back(G);
  array_to_merge.push_back(R);

  // Create a bgr mat
  merge(array_to_merge, bgr);

  // Convert to HSV
  cvtColor(bgr, hsv_image, COLOR_BGR2HSV);

  // Look for both ranges of red
  inRange(hsv_image, Scalar(0, 100, 100), Scalar(10, 255, 255), lower_red_hue_range);
  inRange(hsv_image, Scalar(160, 100, 100), Scalar(179, 255, 255), upper_red_hue_range);

  // Combind and blur
  addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, red_hue_image);
  GaussianBlur(red_hue_image, red_hue_image, Size(9, 9), 2, 2);

  // Find all the contours
  vector<vector<Point> > contours;
  findContours(red_hue_image, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

  int largest_contour_index = 0;
  double largest_area = 0;

  for(size_t i = 0; i < contours.size(); i++) { // iterate through each contour.
    double a = contourArea(contours[i], false);

    if(a>largest_area){
      largest_area=a;
      largest_contour_index=i;                  //Store the index of largest contour
      bounding_rect=boundingRect(contours[i]);  // Find the bounding rectangle for biggest contour
    }
  }

  if (showImg) {
    drawContours(bgr, contours, -1, Scalar(255, 0, 0), 5);
    rectangle(bgr, bounding_rect, Scalar(0, 255, 0), 2, 8, 0);
    imshow("redBox", bgr);
  }
}

int RedBox::getLeft() {
  return bounding_rect.x;
}

int RedBox::getRight() {
  return bounding_rect.x + bounding_rect.width;
}

int RedBox::getTop() {
  return bounding_rect.y;
}

int RedBox::getBottom() {
  return bounding_rect.y + bounding_rect.height;
}
