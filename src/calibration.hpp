#ifndef calibration
#define calibration

#include <libfreenect2/libfreenect2.hpp>
#include <cstdlib>

class RedSquare {
  static const size_t width = 1920;
  static const size_t height = 1080;

  size_t leftWall;
  size_t rightWall;
  size_t topWall;
  size_t bottomWall;


  public:
    size_t leftPos;
    size_t rightPos;
    size_t topPos;
    size_t bottomPos;

    void draw();
    void drawSquare();
    bool moveLeft();
    bool moveRight();
    bool moveUp();
    bool moveDown();
    RedSquare(size_t l, size_t r, size_t t, size_t b);
};

class DepthBox {
  size_t center_of_box;
  float center_value;

  float wall_height;
  float *frame_data;

  private:
    void getCenterValue(libfreenect2::Frame *depth);

  public:
    DepthBox();
    size_t left_wall;
    size_t right_wall;
    size_t top_wall;
    size_t bottom_wall;
    bool calibrated();
    void calibrate(libfreenect2::Frame *depth);
    bool showCalibrationSquare(libfreenect2::Frame *depth);
};
#endif
