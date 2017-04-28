#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <signal.h>
#include <string.h>
#include <fstream>
#include <cstdlib>
#include <math.h>

#include "calibration.hpp"

using namespace cv;
using namespace std;

class MyFileLogger: public libfreenect2::Logger {
  private:
    std::ofstream logfile_;
  public:
    MyFileLogger(const char *filename) {
      if (filename)
        logfile_.open(filename);
      level_ = Debug;
    }
    bool good() {
      return logfile_.is_open() && logfile_.good();
    }
    virtual void log(Level level, const std::string &message) {
      logfile_ << "[" << libfreenect2::Logger::level2str(level) << "] " << message << std::endl;
    }
};

int main() {
  // This code sets up a logger
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
  // avoid flooing the very slow Windows console with debug messages
  libfreenect2::setGlobalLogger(libfreenect2::createConsoleLogger(libfreenect2::Logger::Info));
#else
  // create a console logger with debug level (default is console logger with info level)
  libfreenect2::setGlobalLogger(libfreenect2::createConsoleLogger(libfreenect2::Logger::Debug));
#endif
  MyFileLogger *filelogger = new MyFileLogger(getenv("LOGFILE"));
  if (filelogger->good())
    libfreenect2::setGlobalLogger(filelogger);
  else
    delete filelogger;

  libfreenect2::Freenect2 freenect2;
  libfreenect2::Freenect2Device *dev = 0;
  libfreenect2::PacketPipeline *pipeline = new libfreenect2::OpenGLPacketPipeline();  // Use OpenGL

  std::string serial = "";

  // Find the device
  // Some time dosn't work the frist time
  if(freenect2.enumerateDevices() == 0) {
    std::cout << "no device connected!" << std::endl;
    return -1;
  }
  if (serial == "") {
    serial = freenect2.getDefaultDeviceSerialNumber();
  }

  // Open the device
  dev = freenect2.openDevice(serial, pipeline);

  // Check to make sure it opened correctly
  if(dev == 0) {
    std::cout << "failure opening device!" << std::endl;
    return -1;
  }

  // Set up frame listeners
  int types = libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;
  libfreenect2::SyncMultiFrameListener listener(types);
  libfreenect2::FrameMap frames;

  // Attach them
  dev->setColorFrameListener(&listener);
  dev->setIrAndDepthFrameListener(&listener);

  // Start getting frames
  if (!dev->start()) return -1;

  std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
  std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;


  DepthBox depthBox;
  libfreenect2::Frame *depth;

  while(!depthBox.calibrated()) {
    if (!listener.waitForNewFrame(frames, 10*1000)) {
      std::cout << "timeout!" << std::endl;
      return -1;
    }

    depth = frames[libfreenect2::Frame::Depth];

    depthBox.calibrate(depth);

    listener.release(frames);
  }

  float wConst = 1920.0 / 512.0;
  float hConst = 1080.0 / 424.0;

  size_t tLeft = static_cast <size_t> (floor(depthBox.left_wall * wConst));
  size_t tRight = static_cast <size_t> (floor(depthBox.right_wall * wConst));
  size_t tTop = static_cast <size_t> (floor(depthBox.top_wall * hConst));
  size_t tBot = static_cast <size_t> (floor(depthBox.bottom_wall * hConst));

  while(true) {

    Mat hsv_image, bgr, R, G, B;

    if (!listener.waitForNewFrame(frames, 10*1000)) {
      std::cout << "timeout!" << std::endl;
      return -1;
    }

    libfreenect2::Frame *color = frames[libfreenect2::Frame::Color];

    Mat argb = Mat(cvSize(1920, 1080), CV_8UC4, color->data, Mat::AUTO_STEP);

    extractChannel(argb, B, 0);
    extractChannel(argb, G, 1);
    extractChannel(argb, R, 2);

    std::vector<cv::Mat> array_to_merge;

    array_to_merge.push_back(B);
    array_to_merge.push_back(G);
    array_to_merge.push_back(R);

    merge(array_to_merge, bgr);

    cvtColor(bgr, hsv_image, COLOR_BGR2HSV);

    Mat lower_red_hue_range, upper_red_hue_range, red_hue_image;

    inRange(hsv_image, Scalar(0, 100, 100), Scalar(10, 255, 255), lower_red_hue_range);
    inRange(hsv_image, Scalar(160, 100, 100), Scalar(179, 255, 255), upper_red_hue_range);

    addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, red_hue_image);
    GaussianBlur(red_hue_image, red_hue_image, Size(9, 9), 2, 2);

    vector<vector<Point> > contours;
    Scalar lineColor( 255, 0, 0 );
    findContours(red_hue_image, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    drawContours(bgr, contours, -1, lineColor, 5);

    imshow("image", bgr);
    waitKey(5);

    listener.release(frames);
  }

  dev->stop();
  dev->close();

  return 0;
}
