#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/packet_pipeline.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include <iostream>
#include <signal.h>
#include <string.h>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <cmath>

using namespace cv;
using namespace std;


int thresh = 50, N = 11;

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle( Point pt1, Point pt2, Point pt0 ) {
  double dx1 = pt1.x - pt0.x;
  double dy1 = pt1.y - pt0.y;
  double dx2 = pt2.x - pt0.x;
  double dy2 = pt2.y - pt0.y;
  return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
static void findSquares(const Mat& image, vector<vector<Point> >& squares) {
  squares.clear();

  Mat pyr, timg, gray0(image.size(), CV_8U), gray;

  // down-scale and upscale the image to filter out the noise
  pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
  pyrUp(pyr, timg, image.size());
  vector<vector<Point> > contours;

  // find squares in every color plane of the image
  for (int c = 0; c < 3; c++) {
    int ch[] = {c, 0};
    mixChannels(&timg, 1, &gray0, 1, ch, 1);

    // try several threshold levels
    for (int l = 0; l < N; l++) {
      // hack: use Canny instead of zero threshold level.
      // Canny helps to catch squares with gradient shading
      if (l == 0) {
        // apply Canny. Take the upper threshold from slider
        // and set the lower to 0 (which forces edges merging)
        Canny(gray0, gray, 0, thresh, 5);
        // dilate canny output to remove potential
        // holes between edge segments
        dilate(gray, gray, Mat(), Point(-1,-1));
      } else {
        // apply threshold if l!=0:
        //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
        gray = gray0 >= (l+1)*255/N;
      }

      // find contours and store them all as a list
      findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

      vector<Point> approx;

      // test each contour
      for(size_t i = 0; i < contours.size(); i++) {
        // approximate contour with accuracy proportional
        // to the contour perimeter
        approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

        // square contours should have 4 vertices after approximation
        // relatively large area (to filter out noisy contours)
        // and be convex.
        // Note: absolute value of an area is used because
        // area may be positive or negative - in accordance with the
        // contour orientation
        if(approx.size() == 4 && fabs(contourArea(Mat(approx))) > 1000 && isContourConvex(Mat(approx))) {
          double maxCosine = 0;

          for( int j = 2; j < 5; j++ ) {
            // find the maximum cosine of the angle between joint edges
            double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
            maxCosine = MAX(maxCosine, cosine);
          }

          // if cosines of all angles are small
          // (all angles are ~90 degree) then write quandrange
          // vertices to resultant sequence
          if( maxCosine < 0.3 ) {
            squares.push_back(approx);
          }
        }
      }
    }
  }
}

int main() {
  libfreenect2::Freenect2 freenect2;
  libfreenect2::Freenect2Device *dev = 0;
  libfreenect2::PacketPipeline *pipeline = new libfreenect2::OpenGLPacketPipeline();

  std::string serial = "";

  if(freenect2.enumerateDevices() == 0) {
    std::cout << "no device connected!" << std::endl;
    return -1;
  }
  if (serial == "") {
    serial = freenect2.getDefaultDeviceSerialNumber();
  }

  dev = freenect2.openDevice(serial, pipeline);

  if(dev == 0) {
    std::cout << "failure opening device!" << std::endl;
    return -1;
  }

  int types = libfreenect2::Frame::Depth;
  libfreenect2::SyncMultiFrameListener listener(types);
  libfreenect2::FrameMap frames;

  dev->setColorFrameListener(&listener);
  dev->setIrAndDepthFrameListener(&listener);

  if (!dev->start()) return -1;

  if (!listener.waitForNewFrame(frames, 10*1000)) {
    std::cout << "timeout!" << std::endl;
    return -1;
  }


  libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

  listener.release(frames);

  double min, max;
  Rect foundSquare;

  while(true) {
    Mat box, adjMap, colorMap;
    if (!listener.waitForNewFrame(frames, 10*1000)) { std::cout << "timeout!" << std::endl;
      return -1;
    }

    depth = frames[libfreenect2::Frame::Depth];
    Mat img = Mat(cvSize(depth->width, depth->height), CV_32FC1, depth->data, Mat::AUTO_STEP);

    img.convertTo(adjMap, CV_8UC1, 255.0 / 1600);
    applyColorMap(adjMap, colorMap, COLORMAP_RAINBOW);

    vector<vector<Point> > squares;
    findSquares(colorMap, squares);

    for(size_t i = 0; i < squares.size(); i++) {
      Mat newImage = colorMap.clone();

      Rect bounding_rect = boundingRect(squares[i]);
      rectangle(newImage, bounding_rect, Scalar(0, 0, 255), 2, 8, 0);

      box = img(bounding_rect);
      minMaxLoc(box, &min, &max);

      imshow("Find box", newImage);

      int keyCode = waitKey();
      if (keyCode == 'y') {
        foundSquare = bounding_rect;
        break;
      }
    }

    listener.release(frames);
    if (foundSquare.width != 0) break;
  }

  destroyAllWindows();
  namedWindow("Viewer", WINDOW_NORMAL);

  while (true) {
    Mat box, newMat, scaledBox, flippedMat;

    if (!listener.waitForNewFrame(frames, 10*1000)) {
      std::cout << "timeout!" << std::endl;
      return -1;
    }

    depth = frames[libfreenect2::Frame::Depth];
    Mat img = Mat(cvSize(depth->width, depth->height), CV_32FC1, depth->data, Mat::AUTO_STEP);

    box = img(foundSquare);
    box.convertTo(scaledBox, CV_8UC1, 255.0 / (max - min), -min*(255.0 / (max - min)));
    applyColorMap(scaledBox, newMat, COLORMAP_RAINBOW);

    flip(newMat, flippedMat, 1);

    GaussianBlur(flippedMat, flippedMat, Size(9, 9), 3, 3);

    imshow("Viewer", flippedMat);
    waitKey(5);

    listener.release(frames);
  }

  destroyAllWindows();
  dev->stop();
  dev->close();
}
