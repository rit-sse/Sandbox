#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;

int main() {
  Mat hsv_image, dimg;

  Mat img = imread("/home/dylan/sandbox/box.png");

  cvtColor(img, hsv_image, COLOR_BGR2HSV_FULL);

  imshow("display2", hsv_image);

  inRange(hsv_image, Scalar(25, 50, 125), Scalar(35, 100, 200), dimg);

  GaussianBlur(dimg, dimg, Size(9, 9), 2, 2);

  imshow("display", dimg);

  vector<vector<Point> > contours;

  findContours( dimg, contours, CV_RETR_TREE , CV_CHAIN_APPROX_NONE );

  double largest_area = 0;
  int largest_contour_index = 0;
  Rect bounding_rect;

  for(size_t i = 0; i < contours.size(); i++) { // iterate through each contour.
    double a=contourArea( contours[i],false); 

    if(a>largest_area){
      largest_area=a;
      largest_contour_index=i;                  //Store the index of largest contour
      bounding_rect=boundingRect(contours[i]);  // Find the bounding rectangle for biggest contour
    }
  }

  drawContours( img, contours, largest_contour_index, Scalar(255,0,0), 2 );

  rectangle(img, bounding_rect,  Scalar(0,255,0), 2, 8,0);

  imshow("display2", img);

  waitKey(0);

  return 0;
}
